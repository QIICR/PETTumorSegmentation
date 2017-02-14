/*==============================================================================
 
 Program: PETTumorSegmentation
 
 Portions (c) Copyright University of Iowa All Rights Reserved.
 Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.
 
 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 ==============================================================================*/

/**
\mainpage	PETTumorSegmentation
\date	12/1/2014
\authors	Christian Bauer, Markus Van Tol
\section detail_sec	Description
	This effect is intended for segmentation of lesions in SUV PET volumes.
\n  The effect has multiple options for refinement or adjusting settings.
*/


// PETTumorSegmentation Logic includes
#include "vtkSlicerPETTumorSegmentationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLPETTumorSegmentationParametersNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLSegmentationNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageCast.h>
#include <vtkTypeTraits.h>
#include <vtkSegmentation.h>
#include <vtkOrientedImageData.h>

#include <vtkSlicerSegmentationsModuleLogic.h>

// ITK includes
#include <itkResampleImageFilter.h>
#include <itkRegularSphereMeshSource.h>
#include <itkTriangleMeshToBinaryImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkConstNeighborhoodIterator.h>
#include <itkImageFileWriter.h>
#include <itkMeshFileWriter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkTimeProbe.h>
#include <itkMedianImageFilter.h>

// Optimal Surface Finding includes
#include "itkMeshToOSFGraphFilter.h"
#include "itkLOGISMOSOSFGraphSolverFilter.h"
#include "itkOSFGraphToMeshFilter.h"
#include "itkCloneOSFGraphFilter.h"
#include "itkCenterNormalColumnBuilderFilter.h"
#include "itkSimpleOSFGraphBuilderFilter.h"
#include "itkSealingSegmentationMergerImageFilter.h"
#include "itkWorkers.h"

// STD includes
#include <cassert>
#include <algorithm>
#include <queue>

#include <qSlicerApplication.h>

const int vtkSlicerPETTumorSegmentationLogic::meshResolution = 4;
const float vtkSlicerPETTumorSegmentationLogic::meshSphereRadius = 60.0;
const float vtkSlicerPETTumorSegmentationLogic::columnStepSize = 1.0;
const int vtkSlicerPETTumorSegmentationLogic::hardSmoothnessConstraint = 5;
const float vtkSlicerPETTumorSegmentationLogic::softSmoothnessPenalty = 0.005;
const float vtkSlicerPETTumorSegmentationLogic::softSmoothnessPenaltySplitting = 0.05;
const int vtkSlicerPETTumorSegmentationLogic::minNodeRejections = 3;
const int vtkSlicerPETTumorSegmentationLogic::maxNodeRefinement = 56;
const float vtkSlicerPETTumorSegmentationLogic::rejectionValue = 6.0;
const int vtkSlicerPETTumorSegmentationLogic::numHistogramBins = 100;
const float vtkSlicerPETTumorSegmentationLogic::centeringRange = 7.0;
const int vtkSlicerPETTumorSegmentationLogic::templateMatchingHalfLength = 3;
const float vtkSlicerPETTumorSegmentationLogic::similarityThresholdFactor = 0.05;


//----------------------------------------------------------------------------
/**

*/
vtkStandardNewMacro(vtkSlicerPETTumorSegmentationLogic);

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::vtkSlicerPETTumorSegmentationLogic()
{
  //Clear local variables
  volumeFingerPrint.erase();
  centerFingerPrint.clear();
  StrongWatershedVolume_saved = NULL;
  WeakWatershedVolume_saved = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::~vtkSlicerPETTumorSegmentationLogic()
{
}


//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::Apply(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData)
{
  if (!ValidInput(node))  //check for validity
    return;
  ScalarImageType::Pointer petVolume = GetPETVolume(node);
  LabelImageType::Pointer initialLabelMap(NULL);
  if (labelImageData!=NULL) // for use with Segmentation Editor
    initialLabelMap = ConvertLabelImageToITK(node, labelImageData);
  else // for use with Segment Editor
  {
    node->SetLabel( this->GetSegmentLabel(node) );
    if (node->GetInitialLabelMap().IsNull())
      node->SetInitialLabelMap(this->ConvertSegmentationToITK(node));
    initialLabelMap = resampleNN<LabelImageType,ScalarImageType>(node->GetInitialLabelMap(), petVolume);
  }
  
  //If from a click, there will be a new finger print.  If not, update the finger print.
  if (!this->CheckFingerPrint(node))
  { this->UpdateFingerPrint(node);  }
  
  //Try to initialize graph with standard costs.  It fails if there's no center point or if the center point is misplaced (off the PET volume).
  bool initializeSuccess = InitializeOSFSegmentation(node, petVolume, initialLabelMap);
  if (initializeSuccess)
  {
    UpdateGraphCostsGlobally(node, petVolume, initialLabelMap); //Reapply global refinement, in case apply is from button.  If from click, then there won't be a point anyway.
    UpdateGraphCostsLocally(node, petVolume, true); //Reapply all local refinement, in case apply is from button.  If from click, then there aren't any points anyway.

    //Create the segmentation and apply it to the label map.
    FinalizeOSFSegmentation(node, petVolume, initialLabelMap);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::ApplyGlobalRefinement(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData)
{
  if (!ValidInput(node) || node->GetOSFGraph().IsNull())  //check for validity and graph existence
    return;
  
  ScalarImageType::Pointer petVolume = GetPETVolume(node);  //convert pet volume to ITK for processing
  LabelImageType::Pointer initialLabelMap(NULL);
  if (labelImageData!=NULL) // for use with Segmentation Editor
    initialLabelMap = ConvertLabelImageToITK(node, labelImageData);
  else // for use with Segment Editor
    initialLabelMap = resampleNN<LabelImageType,ScalarImageType>(node->GetInitialLabelMap(), petVolume);
  
  node->SetOSFGraph( Clone(node->GetOSFGraph()) ); // we manipulate graph costs directly; therefore, we need to clone the initial graph to ensure correct undo/redo behavior
  UpdateGraphCostsGlobally(node, petVolume, initialLabelMap); //Sets the cost for all nodes by threshold.  New threshold is determined inside.
  
  UpdateGraphCostsLocally(node, petVolume, true); //Reapplies all local refinement, since older points' effects are lost when global update changes base cost.
  FinalizeOSFSegmentation(node, petVolume, initialLabelMap);  //Applies the changed label map
  
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::ApplyLocalRefinement(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData)
{
  if (!ValidInput(node) || node->GetOSFGraph().IsNull())  //check for validity and graph existence
    return;
  
  ScalarImageType::Pointer petVolume = GetPETVolume(node);  //convert pet volume to ITK for processing
  LabelImageType::Pointer initialLabelMap(NULL);
  if (labelImageData!=NULL) // for use with Segmentation Editor
    initialLabelMap = ConvertLabelImageToITK(node, labelImageData);
  else // for use with Segment Editor
    initialLabelMap = resampleNN<LabelImageType,ScalarImageType>(node->GetInitialLabelMap(), petVolume);
  
  node->SetOSFGraph( Clone(node->GetOSFGraph()) ); // we manipulate graph costs directly; therefore, we need to clone the initial graph to ensure correct undo/redo behavior
  UpdateGraphCostsLocally(node, petVolume); //Add effect of most recent refinement point only

  FinalizeOSFSegmentation(node, petVolume, initialLabelMap);  //Applies the changed label map

}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::LabelImageType::Pointer vtkSlicerPETTumorSegmentationLogic::ConvertLabelImageToITK(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData)
{
  //labelImageData, which may be of an older state, doesn't contain the spacing/origin information, so we have to retrieve it from the most recent segmentation label volume
  //Verification that labelImageData and the segmentation label volume here refer to states of the same region is done earlier, in the editor effect portion.
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  vtkMRMLScalarVolumeNode* vtkLabelVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationVolumeReference() ));
  
  //Convert the image data from labelImageData, then set the spacing and origin directly, then return the ITK label image.
  LabelImageType::Pointer labelVolume = convert2ITK<LabelImageType>( labelImageData );
  labelVolume->SetSpacing( vtkLabelVolume->GetSpacing() );
  double origin2[3] = {-vtkLabelVolume->GetOrigin()[0], -vtkLabelVolume->GetOrigin()[1], vtkLabelVolume->GetOrigin()[2]};
  labelVolume->SetOrigin( origin2 );
  return labelVolume;
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::LabelImageType::Pointer vtkSlicerPETTumorSegmentationLogic::ConvertSegmentationToITK(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  vtkMRMLSegmentationNode* vtkSegmentation = static_cast<vtkMRMLSegmentationNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationReference() ));
  vtkMRMLScalarVolumeNode* vtkPetVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetPETVolumeReference() ));
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry = 
    vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(vtkPetVolume);
  
  vtkSmartPointer<vtkOrientedImageData> vtkLabelVolume = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkSegmentation->GenerateMergedLabelmapForAllSegments(vtkLabelVolume, vtkSegmentation::EXTENT_UNION_OF_SEGMENTS_PADDED, referenceGeometry);
  
  LabelImageType::Pointer labelVolume = convert2ITK<LabelImageType>( vtkLabelVolume );
  labelVolume->SetSpacing( vtkLabelVolume->GetSpacing() );
  double origin2[3] = {-referenceGeometry->GetOrigin()[0], -referenceGeometry->GetOrigin()[1], referenceGeometry->GetOrigin()[2]};
  labelVolume->SetOrigin( origin2 );

  referenceGeometry->Delete();
  
  return labelVolume;
}

//----------------------------------------------------------------------------
short vtkSlicerPETTumorSegmentationLogic::GetSegmentLabel(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  vtkMRMLSegmentationNode* vtkSegmentation = static_cast<vtkMRMLSegmentationNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationReference() ));
  std::vector< std::string > segmentIds;
  vtkSegmentation->GetSegmentation()->GetSegmentIDs(segmentIds);
  short label = -1;
  for (size_t i=0; i<segmentIds.size(); i++)
    if (segmentIds[i]==node->GetSelectedSegmentID())
      label = i;
  if(label == -1)
    std::cout << "Error: Failed to find " << node->GetSelectedSegmentID() << "in the segmentation" << std::endl;
  return label+1;
}

//----------------------------------------------------------------------------
bool vtkSlicerPETTumorSegmentationLogic::ValidInput(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  // verify centerpoint
  vtkMRMLFiducialListNode* centerFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetCenterPointIndicatorListReference()) );
  if  (centerFiducials==0 || centerFiducials->GetNumberOfFiducials()==0)
    return false;
    
  // verify existance of pet scan
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  vtkMRMLScalarVolumeNode* petVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetPETVolumeReference() ));
  if ( petVolume==NULL )
    return false;
  
  // verify existance of label map or segmentation
  vtkMRMLScalarVolumeNode* segmentationVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationVolumeReference() ));
  vtkMRMLSegmentationNode* segmentation = static_cast<vtkMRMLSegmentationNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationReference() ));
  if ( segmentation==NULL && segmentationVolume==NULL )
    return false;    
    
  // verify validity of centerpoint
  //  done in a separate step, requires other input
  
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerPETTumorSegmentationLogic::InitializeOSFSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap)
{
  //Determine the center point (and in doing so verify it's location)
  if (CalculateCenterPoint(node, petVolume, initialLabelMap))
  {
    CreateGraph(node);
    ObtainHistogram(node, petVolume);
    return true;
  }
  else
  {
    //Remove most recent point if it failed.  This prevents it from being kept in memory or showing up as a debug info point in Slicer.
    vtkMRMLFiducialListNode* centerFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetCenterPointIndicatorListReference()) );
    if  (centerFiducials==0 || centerFiducials->GetNumberOfFiducials()==0)
      return false;
    else
    {
      centerFiducials->RemoveFiducial(centerFiducials->GetNumberOfFiducials()-1);
      return false;
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::FinalizeOSFSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap)
{  
  //Run the maximum flow algorithm
  MaxFlow(node); 
  
  //Get the resulting boundary it determined
  MeshType::Pointer segmentationMesh = GetSegmentationMesh(node);
  
  //Voxelize that boundary
  LabelImageType::Pointer segmentation = GetSegmentation(node, segmentationMesh, initialLabelMap); 
  
  //Integrate that segmentation with the existing segmentation
  UpdateOutput(node, petVolume, segmentation, initialLabelMap);

}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::UpdateGraphCostsGlobally(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap)
{
  vtkMRMLFiducialListNode* globalRefinementFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetGlobalRefinementIndicatorListReference()) );
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if ( globalRefinementFiducials==0 || petVolume.IsNull() || initialLabelMap.IsNull() || graph.IsNull())
    return;
 
  //If there are no global refinement points, calculate threshold automatically
  if (globalRefinementFiducials->GetNumberOfFiducials()==0)
    CalculateThresholdHistogramBased(node, petVolume);
  else //Otherwise, get it by the point
    CalculateThresholdPointLocationBased(node, petVolume);
  
  //Create the interpolators for the volumes here to avoid instantiating new ones for every thread.
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  interpolator->SetInputImage( petVolume );
  LabelInterpolatorType::Pointer labelInterpolator = LabelInterpolatorType::New();
  labelInterpolator->SetInputImage( initialLabelMap );
  WatershedInterpolatorType::Pointer strongWatershedInterpolator = WatershedInterpolatorType::New();
  strongWatershedInterpolator->SetInputImage( GetStrongWatershedVolume(node, petVolume) );
  WatershedInterpolatorType::Pointer weakWatershedInterpolator = WatershedInterpolatorType::New();
  weakWatershedInterpolator->SetInputImage( GetWeakWatershedVolume(node, petVolume) );
   
  //Multithreaded graph cost setting.
  int numVertices = node->GetOSFGraph()->GetSurface()->GetNumberOfVertices();
  itk::Workers().RunFunctionForRange<int, vtkMRMLPETTumorSegmentationParametersNode*, InterpolatorType::Pointer, LabelInterpolatorType::Pointer, WatershedInterpolatorType::Pointer, WatershedInterpolatorType::Pointer>
    (&SetGlobalGraphCostsForVertex, 0, numVertices-1, node, interpolator, labelInterpolator, strongWatershedInterpolator, weakWatershedInterpolator);
  
  //If there's a global refinement point, apply the specific cost effect of it on the relevant column (cost +1000 to all nodes on the column but closest node to point)
  if (globalRefinementFiducials->GetNumberOfFiducials()!=0)
  {
    // adjust cost function for column closest to refinement point
    PointType refinementPoint = convert2ITK( globalRefinementFiducials->GetNthFiducialXYZ(globalRefinementFiducials->GetNumberOfFiducials()-1) );
    int vertexId = GetClosestVertex(node, refinementPoint);
    int columnId = GetClosestColumnOnVertex(node, refinementPoint, vertexId);
    std::vector<float>& costs = graph->GetSurface()->GetColumnCosts(vertexId)->CastToSTLContainer();
    for (size_t i=0; i<costs.size(); i++)
      costs[i]+=1000;
    costs[columnId]-=1000;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::SetGlobalGraphCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, InterpolatorType::Pointer interpolator, LabelInterpolatorType::Pointer labelInterpolator, WatershedInterpolatorType::Pointer strongWatershedInterpolator, WatershedInterpolatorType::Pointer weakWatershedInterpolator)
{
  //Determine and store the uptake values on the nodes, since they are used very frequently.  Cuts down on interpolator access.
  const std::vector<float> uptakeValues = SampleColumnPoints<float, InterpolatorType>(vertexId, node, interpolator);
  
  SetGlobalBaseGraphCostsForVertex(vertexId, node, uptakeValues); //Set the costs based on the threshold, as well as the standard rejection
  if (!node->GetPaintOver())
    AddLabelAvoidanceCostsForVertex(vertexId, node, uptakeValues, labelInterpolator); //Adds the costs to reject other objects
  else if (node->GetNecroticRegion())
    AddDefaultNecroticCostsForVertex(vertexId, node, labelInterpolator);  //Adds the costs for necrotic mode, if it is active and label avoidance is not
  if (node->GetSplitting())
    AddSplittingCostsForVertex(vertexId, node, uptakeValues, strongWatershedInterpolator, weakWatershedInterpolator); //Adds the costs for splitting, if active 
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::SetGlobalBaseGraphCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues)
{
  // get parameters
  const std::vector<float>& histogram = node->GetHistogram();
  float histogramRange = node->GetHistogramRange();
  float threshold = node->GetThreshold();
  float lowerBound = node->GetHistogramMedian();
  float centerpointUptake = node->GetCenterpointUptake();
  bool necroticRegion = node->GetNecroticRegion();
  bool linearCost = node->GetLinearCost();

  // calculate base cost
  std::vector<float> costs(uptakeValues.size(), 1.0);
  for (size_t i=0; i<costs.size(); i++)
  {
    float uptake = uptakeValues[i];
    float cost = 0.0;
    
    if (uptake<threshold && !linearCost)
    { //cost below threshold w/o linear cost indexes into the histogram
      int index = (int) ((uptake / histogramRange) * histogram.size());
      index = std::max( std::min(index, int(numHistogramBins)-1), 0);
      cost = histogram[index];
    }
    else if (uptake<threshold && linearCost)
      cost = 1.0 - (uptake / threshold);  //cost below threshold w/ linear cost is a linear with cost 1 at uptake 0 and cost 0 at uptake of the threshold
    else if (uptake==threshold) //cost is 0 at the threshold
      cost = 0.0;
    else if (uptake>threshold && centerpointUptake>threshold)
      cost = (uptake-threshold)/(centerpointUptake-threshold);  //cost above threshold w/ center above threshold is linear with cost 1 at uptake of the center and cost 0 at uptake of the threshold
    else
      cost = 1.0; //cost above threshold with a center value below the threshold breaks the linear function, so it's just 1.0 by default
    costs[i] = cost;
  }
  
  // add rejections
  bool belowMin = false;
  bool aboveThres = true;
  if (necroticRegion == true) //if necrotic, uptake must first go above threshold before it can be checked as being below the minimum
  { aboveThres = false; }

  //rejection
  for (size_t i=0; i<costs.size(); i++)
  {
    if (i<size_t(minNodeRejections)) // too close to center
      costs[i]+=rejectionValue;

    if (necroticRegion && uptakeValues[i]>threshold)  //mark above threshold when necrotic mode is active
      aboveThres = true;
    if (aboveThres && uptakeValues[i]<lowerBound) // uptake too low, reject any beyond in order to avoid including outside objects
      belowMin = true;
    if (belowMin && i>size_t(minNodeRejections))  // rejection applied, even if uptake returns above minimum value
      costs[i]+=rejectionValue;    
  }
  
  // set costs for vertex
  typedef OSFSurfaceType::ColumnCostsContainer ColumnCostsContainer;
  ColumnCostsContainer::Pointer columnCosts = node->GetOSFGraph()->GetSurface()->GetColumnCosts( vertexId );
  columnCosts->CastToSTLContainer() = costs;
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::AddLabelAvoidanceCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues, LabelInterpolatorType::Pointer labelInterpolator)
{
/*
Requirements:
1. Reject immediately upon encountering another label, except for the first node available
2. Do not apply rejection in addition to existing rejection
3. Apply cost sealing condition
4. Recognize effects of necrotic mode
*/

  // get parameters
  float threshold = node->GetThreshold();
  float lowerBound = node->GetHistogramMedian();
  int label = node->GetLabel();
  const std::vector<short> labelValues = SampleColumnPoints<short, LabelInterpolatorType>(vertexId, node, labelInterpolator);
  std::vector<float>& costs = node->GetOSFGraph()->GetSurface()->GetColumnCosts( vertexId )->CastToSTLContainer();
  bool necroticRegion = node->GetNecroticRegion();

  // add rejections
  bool labelChanged = false;
  bool belowMin = false;
  bool aboveThres = true;
  if (necroticRegion == true)
  { aboveThres = false; }

  for (size_t i=0; i<costs.size(); i++)
  {
    //detect label changed, signalling need to reject
    if (labelValues[i]!=0 && labelValues[i]!=label)
      labelChanged = true;

    //determine if already rejected
    if (necroticRegion && uptakeValues[i]>threshold)
      aboveThres = true;
    if (aboveThres && uptakeValues[i]<lowerBound) // uptake too low
      belowMin = true;

    //if label requires rejection AND not already rejected AND not on unrejectable node, reject
    if ( labelChanged && !belowMin && i>size_t(minNodeRejections))
      costs[i]+=rejectionValue;   
  }

  // check cost seal condition
  labelChanged = false;
  int nodeToSeal = -1;
  bool doNotSeal = false;
  int firstCheckedNode = vtkSlicerPETTumorSegmentationLogic::minNodeRejections-1;

  // apply necrotic sealing condition, and determine starting node for cost sealing condition by necrotic mode
  if (necroticRegion == true)
  {
    int j = 0;
    //search for where the uptake first rises above the threshold
    while (j < int(uptakeValues.size()) && uptakeValues[j] < threshold)
    { j++;  }
    //mark the proper node 
    if (firstCheckedNode < j) //firstCheckedNode == j_{Th_{i}} in the thesis.
      firstCheckedNode = j;
    
    //go along the column from the center and find out if there's a node to seal to for necrotic mode
    for (size_t i=vtkSlicerPETTumorSegmentationLogic::minNodeRejections; i<costs.size()-1; i++)
    {
      //search in first set of zero labels
      if (labelValues[i] == 0)
      {
        //if i is in proper region to leave necrotic mode, done trying to seal on this column
        if (int(i)-1 > firstCheckedNode && uptakeValues[i] < uptakeValues[i-1])  //i-1 = j', i = j'' in the thesis.  This checks the case to cancel the necrotic sealing condition.
        { i = costs.size()-1; }
        else if (labelValues[i+1] == label) //otherwise, if next node is the sought label, seal to it and end the search
        {
          nodeToSeal = i;
          i = costs.size()-1;
        }
        else if (labelValues[i+1] != 0) //otherwise, if the next node is some other label, do not seal to it; end the search
        { i = costs.size()-1; }
      }
      else  //if already at a nonzero label, done trying to seal
      { i = costs.size()-1; }
    }
  }

  //apply base cost seal condition, if not already sealed by necrotic sealing condition
  if (nodeToSeal == -1)
  {   
    for (size_t i=vtkSlicerPETTumorSegmentationLogic::minNodeRejections; i<size_t(costs.size()) && nodeToSeal < 0 && doNotSeal == false; i++)
    {
      if (labelValues[i]!=0 && labelValues[i]!=label && (int(i) < firstCheckedNode || int(i) < minNodeRejections)) // Prevents sealing if earlier labels occur before leaving the close rejected region or, in necrotic mode, the necrotic region
      { doNotSeal = true; }

      //continues incrementing the node to seal if there is no decrease yet.
      //If there's another label on current node, then seal here; the previous i value (at minNodeRejections or higher) has already been checked by the next if statement, as verified by doNotSeal being false.
      if (int(i) > minNodeRejections && int(i) >= firstCheckedNode && labelValues[i]!=0 && labelValues[i]!=label)
      {
        if (doNotSeal == false)
        { nodeToSeal = i-1; }      
      }
      if (int(i) > firstCheckedNode && uptakeValues[i] < uptakeValues[i-1] && labelChanged == false)  //for necrotic mode, do not check until node and previous are both after the first node above the threshold; j_Th < j' < j'' < j
      { doNotSeal = true; }
    }
  }

  // seal if the cost seal condition is met
  if (nodeToSeal != -1 && nodeToSeal >= vtkSlicerPETTumorSegmentationLogic::minNodeRejections)
  {
    float sealingNotch = 2.0;
    float sealingSigma = 1.0;
    int sealingNodeLimit = 6;
    for (int i=0; i<=nodeToSeal; i++)
    { 
      if (i <= nodeToSeal && nodeToSeal - i <= sealingNodeLimit && i >= 0)  //limit nodes affected to those that are within the sealing node limit and those that are of existing indices
      { costs[i] -= sealingNotch * std::exp( -(float) ((float) i-(float) nodeToSeal)*((float) i-(float) nodeToSeal) / (2*sealingSigma*sealingSigma) );  }
    }
  }


}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::AddDefaultNecroticCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, LabelInterpolatorType::Pointer labelInterpolator)
{
  //default necrotic costs seal to the first matching label
  int label = node->GetLabel();
  const std::vector<short> labelValues = SampleColumnPoints<short, LabelInterpolatorType>(vertexId, node, labelInterpolator);
  std::vector<float>& costs = node->GetOSFGraph()->GetSurface()->GetColumnCosts( vertexId )->CastToSTLContainer();

  int nodeToSeal = -1;
  
  for (size_t i=0; i<costs.size()-1; i++)
  {
    //find first node where next label is ours when current label is background
    if (labelValues[i] == 0)
    {
      if (labelValues[i+1] == label)
      { nodeToSeal = i; }
      else if (labelValues[i+1] != 0)
      { i = costs.size()-1; }
    }
  }

  // seal if the necrotic seal condition is met
  if (nodeToSeal != -1)
  {
    float sealingNotch = 2.0;
    float sealingSigma = 1.0;
    int sealingNodeLimit = 6;
    for (int i=minNodeRejections; i<=nodeToSeal; i++)
    {
      if (i <= nodeToSeal && nodeToSeal - i <= sealingNodeLimit && i >= 0)  //limit nodes affected to those that are within the sealing node limit and those that are of existing indices
      { costs[i] -= sealingNotch * std::exp( -(float) ((float) i-(float) nodeToSeal)*((float) i-(float) nodeToSeal) / (2*sealingSigma*sealingSigma) );  }
    }
  }
  
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::AddSplittingCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues, WatershedInterpolatorType::Pointer strongWatershedInterpolator, WatershedInterpolatorType::Pointer weakWatershedInterpolator)
{
  /*
  Requirements:
  1. Cost change for local minimum of uptake along columns
  2. Cost change for change in strong watershed
  3. Cost change for change in weak watershed
  4. All those costs limited to first region above threshold
  5. Add bias costs
  */

  float threshold = node->GetThreshold();
  const std::vector<WatershedPixelType> strongWatershedValues = SampleColumnPoints<WatershedPixelType, WatershedInterpolatorType>(vertexId, node, strongWatershedInterpolator);
  const std::vector<WatershedPixelType> weakWatershedValues = SampleColumnPoints<WatershedPixelType, WatershedInterpolatorType>(vertexId, node, weakWatershedInterpolator);
  std::vector<float>& costs = node->GetOSFGraph()->GetSurface()->GetColumnCosts( vertexId )->CastToSTLContainer();

  float sigma = 2.0;
  bool anyFeature = false;
  //Sought features: local minimum in uptake along the column, change in strong watershed along the column, change in weak watershed along the column
  //These are only tracked in the continuous set of nodes above or equal to the threshold that is closest to node 0.
  //At each one, apply a cost decrease around the feature, centered at the minimum or just before just after the watershed change.
  for (int i = 0; i < int(costs.size()) && uptakeValues[i] >= threshold; i++)
  {
    if (i > 0 && i < int(costs.size())-1 && uptakeValues[i] < uptakeValues[i-1] && uptakeValues[i] <= uptakeValues[i+1])
    { //local minimum found
      anyFeature = true;
      for (int j = -10; j<=10; j++) //affected region
      {
        if (i+j >=0 && i+j < int(costs.size())-1)
        { costs[i+j] += -0.4 * ( std::exp( -1.0 * (float) j * (float) j / (2.0 * sigma * sigma) )); }
      }
    }
    if (i > 0 && strongWatershedValues[i-1] != strongWatershedValues[i])
    { //strong watershed change found
      anyFeature = true;
      for (int j = -10; j<=10; j++) //affected region
      {
        if (i+j >=0 && i+j < int(costs.size())-1)
        { costs[i+j] += -0.2 * ( std::exp( -1.0 * (float) j * (float) j / (2.0 * sigma * sigma) )); }
      }
    }
    if (i > 0 && weakWatershedValues[i-1] != weakWatershedValues[i])
    { //weak watershed change found
      anyFeature = true;
      for (int j = -10; j<=10; j++) //affected region
      {
        if (i+j >=0 && i+j < int(costs.size())-1)
        { costs[i+j] += -0.5 * ( std::exp( -1.0 * (float) j * (float) j / (2.0 * sigma * sigma) )); }
      }
    }
  }
  if (anyFeature == true)
  { //if any features were found, apply a linear bias to use the closer features first
    for (size_t i = 0; i < costs.size(); i++)
    { costs[i] += (float) ((float) (i+1.0) / 60.0); }
  }
  
}

//----------------------------------------------------------------------------
template <typename valueType, class ImageInterpolatorType>
std::vector<valueType>
vtkSlicerPETTumorSegmentationLogic::SampleColumnPoints(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, typename ImageInterpolatorType::Pointer interpolator, valueType defaultValue)
{
  //Copy all the uptake values interpolated on the column's nodes into a vector.
  typedef OSFSurfaceType::ColumnCoordinatesContainer ColumnCoordinatesContainer;
  
  ColumnCoordinatesContainer::ConstPointer columnCoordinates = node->GetOSFGraph()->GetSurface()->GetColumnCoordinates( vertexId );
  ColumnCoordinatesContainer::ConstIterator coordItr = columnCoordinates->Begin();
  ColumnCoordinatesContainer::ConstIterator coordEnd = columnCoordinates->End();
  std::vector<valueType> values( columnCoordinates->Size(), defaultValue );
  size_t i = 0;
  while (coordItr!=coordEnd)
  {
    if ( interpolator->IsInsideBuffer(coordItr.Value()) )
      values[i] = interpolator->Evaluate(coordItr.Value());
    ++coordItr; ++i;
  }
  
  return values;  
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::UpdateGraphCostsLocally(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, bool renewOldPoints)
{  
  vtkMRMLFiducialListNode* localRefinementFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetLocalRefinementIndicatorListReference()) );
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if (localRefinementFiducials->GetNumberOfFiducials()==0 || graph.IsNull()) // nothing to do
    return;
//  vtkMRMLFiducialListNode* globalRefinementFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetGlobalRefinementIndicatorListReference()) );
//TODO: fill in so depth0ModifiedOverall also tracks any global refinement point and prevents modification thereof
  
  //prevent modification of depth=0 modified columns; IE if a column has a refinement node on it, it can't be modified any further.
  std::vector<bool> depth0ModifiedOverall;
  std::vector<bool> depth0ModifiedSequence;
  depth0ModifiedOverall.resize(node->GetOSFGraph()->GetSurface()->GetNumberOfVertices(), false);
  depth0ModifiedSequence.resize(node->GetOSFGraph()->GetSurface()->GetNumberOfVertices(), false);
  
  //Find the closest vertex id for each of the existing nodes and mark it as modified at depth 0
  for (int i=0; i<localRefinementFiducials->GetNumberOfFiducials(); ++i)
  {
    PointType refinementPoint = convert2ITK( localRefinementFiducials->GetNthFiducialXYZ(i) );
    int vertexId = GetClosestVertex(node, refinementPoint);
    depth0ModifiedOverall[vertexId] = true;
  }

  
  for (int i=(renewOldPoints == true)? 0 : localRefinementFiducials->GetNumberOfFiducials()-1; i<localRefinementFiducials->GetNumberOfFiducials(); ++i)
  {
    PointType refinementPoint = convert2ITK( localRefinementFiducials->GetNthFiducialXYZ(i) );
    AddLocalRefinementCosts(node, petVolume, refinementPoint, depth0ModifiedOverall, depth0ModifiedSequence);
    int vertexId = GetClosestVertex(node, refinementPoint);
    depth0ModifiedSequence[vertexId] = true;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::AddLocalRefinementCosts(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, const PointType& refinementPoint, std::vector<bool> depth0ModifiedOverall, std::vector<bool> depth0ModifiedSequence)
{
  OSFSurfaceType::Pointer surface = node->GetOSFGraph()->GetSurface();
  surface->BuildNeighborLookupTable();
  
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  interpolator->SetInputImage( petVolume );
  
  // find node closest to refinement point and uptake values for template matching
  int vertexId = GetClosestVertex(node, refinementPoint);
  int columnId = GetClosestColumnOnVertex(node, refinementPoint, vertexId);
  if (columnId < minNodeRejections)
    columnId = minNodeRejections;
  if (columnId > maxNodeRefinement)
    columnId = maxNodeRefinement;
  std::vector<float> uptakeValues = SampleColumnPoints<float, InterpolatorType>(vertexId, node, interpolator);
  
  // get similarity threshold
  float similarityThreshold = 0.0;
  for (int i=columnId-templateMatchingHalfLength; i<=columnId+templateMatchingHalfLength; i++)
    if (i>=0 && i<int(uptakeValues.size()))
      similarityThreshold += fabs(uptakeValues[i]);

  
  similarityThreshold *= similarityThresholdFactor;

  // find nearby columns within given range and store additionally obtained information
  const int maxDistance = 5;
  std::vector<int> vertexInRange;
  std::vector<int> vertexMostSimilarColumnId;
  std::vector<int> vertexDistance;
  std::vector<bool> vertexMarked;
  vertexInRange.push_back(vertexId);
  vertexDistance.push_back(0);
  vertexMostSimilarColumnId.push_back(columnId);
  vertexMarked.push_back(true);
  
  std::queue<int> queue;
  queue.push(vertexId);
  while (!queue.empty())
  {
    int vertexId = queue.front();
    queue.pop();
    int id = std::find(vertexInRange.begin(), vertexInRange.end(), vertexId ) - vertexInRange.begin();
    int distance = vertexDistance[id];
    if (distance>=maxDistance) // all unprocessed neighbors are too far away from the center
      continue;
    std::vector<long unsigned int> neighbors = surface->GetNeighbors(vertexId)->CastToSTLConstContainer();
    for (size_t i=0; i<neighbors.size(); ++i)
    {
      int neighborVertexId = neighbors[i];
      if (std::find(vertexInRange.begin(), vertexInRange.end(), neighborVertexId)!=vertexInRange.end()) // already processed this vertex
        continue;
        
      // find most similar uptake vector and obtain similarity meaure
      const std::vector<float> neighborUptakeValues = SampleColumnPoints<float, InterpolatorType>(neighborVertexId, node, interpolator);
      float similarity;
      int bestMatchColumnId = GetBestTemplateMatch(uptakeValues, columnId, templateMatchingHalfLength, neighborUptakeValues, distance+1, similarity);  

      // store surface finding result
      vertexInRange.push_back(neighborVertexId);
      vertexDistance.push_back(distance+1);
      vertexMostSimilarColumnId.push_back(bestMatchColumnId);
      vertexMarked.push_back(similarity<similarityThreshold);// && vertexMarked[id]);
      if (similarity < similarityThreshold)
        queue.push(neighborVertexId);
    }
  }
  
  // mark unmarked columns surrounded mostly by marked columns  
  std::vector<bool> vertexMarkedSealed = vertexMarked;
  for (size_t i=0; i<vertexMarkedSealed.size(); ++i)
  {
    if (!vertexMarkedSealed[i]) // test if most surrounding columns are marked
    {
      int vertexId = vertexInRange[i];
      std::vector<long unsigned int> neighbors = surface->GetNeighbors(vertexId)->CastToSTLConstContainer();
      int numMarkedNeighbors = 0;
      for (size_t j=0; j<neighbors.size(); ++j)
      {
        std::vector<int>::iterator it = std::find(vertexInRange.begin(), vertexInRange.end(), neighbors[j] );
        if (it!=vertexInRange.end() && vertexMarked[it-vertexInRange.begin()])
        { ++numMarkedNeighbors; }
      }
      if ((numMarkedNeighbors>=4 && neighbors.size() == 6) || (numMarkedNeighbors>=3 && neighbors.size() == 4)) // if 2/3rd or neighbors are marked, assuming there are 6 neighbors in the mesh, then also mark this column  (there aren't always 6 neighbors)
      { vertexMarkedSealed[i] = true; }
    }
  }  
  
  // change costs for center vertex
  std::vector<float>& costs = surface->GetColumnCosts(vertexId)->CastToSTLContainer();
  if (!depth0ModifiedSequence[vertexId])
  {
    for (size_t i=0; i<costs.size(); i++)
      costs[i]+=1000;
    costs[columnId]-=1000;
  }
    
  // change costs for all other marked vertices
  for (size_t i=1; i<vertexMarked.size(); ++i)
  {
    if ((!vertexMarked[i] && !vertexMarkedSealed[i]) || depth0ModifiedOverall[vertexInRange[i]])
      continue;


    std::vector<float>& costs = surface->GetColumnCosts(vertexInRange[i])->CastToSTLContainer();
    int columnId = vertexMostSimilarColumnId[i];
    float distance = vertexDistance[i];
    
    for (int j=0; j<int(costs.size()); ++j)
    { costs[j] -= 3.0*exp( -(columnId-j)*(columnId-j)/(2.0*distance*distance) );  }
  }  
  
  // TODO: remove
  if (false) // for debugging
  {
  int numMarked = 0;
  for (size_t i=0; i<vertexMarked.size(); ++i)
    numMarked += vertexMarked[i] ? 1 : 0;
  int numMarkedSealed = 0;
  for (size_t i=0; i<vertexMarkedSealed.size(); ++i)
    numMarkedSealed += vertexMarkedSealed[i] ? 1 : 0;
  }
}

//----------------------------------------------------------------------------
int vtkSlicerPETTumorSegmentationLogic::GetBestTemplateMatch(std::vector<float> vecA, int idxA, int len, std::vector<float> vecB, int searchRange, float& matchingScore)
{
  // obtain vector A
  std::vector<float> a(2*len+1, 0);
  for (int i=idxA-len; i<=idxA+len; i++)
    if (i>=0 && i<int(vecA.size()))
      a[i-idxA+len] = vecA[i];
  // obtain scores for all vectors in search range
  std::vector<float> scores;
  for (int idxB=idxA-searchRange; idxB<=idxA+searchRange; idxB++)
  {
    if (idxB < minNodeRejections || idxB > maxNodeRefinement)
    { scores.push_back(itk::NumericTraits<float>::max()); }
    else
    {
      // obtain vector B
      std::vector<float> b(2*len+1, 0);
      for (int i=idxB-len; i<=idxB+len; i++)
        if (i>=0 && i<int(vecB.size()))
          b[i-idxB+len] = vecB[i];
      
      // calculate matching score
      float score = 0.0;
      for (size_t i=0; i<a.size(); i++)
        score += fabs(a[i]-b[i]);
      
      scores.push_back(score);
    }
  }
  
  // find the best among all scores
  std::vector<float>::const_iterator bestScoreIt = std::min_element(scores.begin(), scores.end());
  matchingScore = *bestScoreIt;
  return idxA-searchRange+(bestScoreIt-scores.begin());
}


//----------------------------------------------------------------------------
bool vtkSlicerPETTumorSegmentationLogic::CalculateCenterPoint(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer labelVolume)
{
  // get centerpoint
  vtkMRMLFiducialListNode* centerFiducials = static_cast<vtkMRMLFiducialListNode*>(node->GetScene()->GetNodeByID( node->GetCenterPointIndicatorListReference() ));
  if (centerFiducials->GetNumberOfFiducials()==0)
    return false;

  
  // check that point is within region of pet and label volumes, and not right at the edges, due to problems that causes.
  ScalarImageType::RegionType petRegion = petVolume->GetLargestPossibleRegion();
  LabelImageType::RegionType labelRegion = labelVolume->GetLargestPossibleRegion();
  
  
  //Make sure the initial point given by each is within the bounds of each.
  
  //Determine the highest and lowest indices in the pet and label images.
  ScalarImageType::IndexType petLowestIndex = petRegion.GetIndex();
  ScalarImageType::IndexType petHighestIndex = petRegion.GetIndex();
  petHighestIndex[0] += petRegion.GetSize()[0]-1;
  petHighestIndex[1] += petRegion.GetSize()[1]-1;
  petHighestIndex[2] += petRegion.GetSize()[2]-1;
  
  ScalarImageType::IndexType labelLowestIndex = labelRegion.GetIndex();
  ScalarImageType::IndexType labelHighestIndex = labelRegion.GetIndex();
  labelHighestIndex[0] += labelRegion.GetSize()[0]-1;
  labelHighestIndex[1] += labelRegion.GetSize()[1]-1;
  labelHighestIndex[2] += labelRegion.GetSize()[2]-1;
  
  //Get the initial point as ITK.
  PointType initialPoint = convert2ITK( centerFiducials->GetNthFiducialXYZ(centerFiducials->GetNumberOfFiducials()-1) );
  node->SetCenterpoint(initialPoint);
  
  LabelImageType::IndexType centerIndex;
  petVolume->TransformPhysicalPointToIndex( initialPoint, centerIndex );
  
  //Check for bounds on each 
  for (int dim = 0; dim < 3; dim++)
  {
    if (centerIndex[dim] < petLowestIndex[dim] || centerIndex[dim] < labelLowestIndex[dim])
    { return false; }
    if (centerIndex[dim] > petHighestIndex[dim] || centerIndex[dim] > labelHighestIndex[dim])
    { return false; }
  }
  
  // if no assist centering, then use the initial point and be done with this section
  if (node->GetAssistCentering()==false)
    return true;
  
  //Otherwise, adjust the center point location
    
  bool paintOver =  node->GetPaintOver();
    
  // get ROI for center point search
  ScalarImageType::RegionType roi;
  roi.SetIndex( centerIndex );
  float minSpacing = std::min( std::min( petVolume->GetSpacing()[0], petVolume->GetSpacing()[1]), petVolume->GetSpacing()[2] );
  roi.PadByRadius( std::ceil( centeringRange/minSpacing ) );
  ScalarImageType::RegionType finalROI = petVolume->GetLargestPossibleRegion();
  finalROI.Crop(roi);
  
  // get new centerpoint as highest uptake point in search area
  itk::ImageRegionIteratorWithIndex<ScalarImageType> it(petVolume, finalROI);
  itk::ConstNeighborhoodIterator<LabelImageType>::RadiusType radius;
  radius.Fill(1);
  itk::ConstNeighborhoodIterator<LabelImageType> lit(radius, labelVolume, finalROI);
  
  const float centeringRangeSquared = centeringRange*centeringRange;
  ScalarImageType::PointType point;
  LabelImageType::PixelType safeLabel = labelVolume->GetPixel(centerIndex);

  float bestUptake = itk::NumericTraits<float>::min();
  while (!it.IsAtEnd())
  {
    petVolume->TransformIndexToPhysicalPoint(it.GetIndex(), point);

    //Check if voxel is in "safe" region, if not overwriting
    bool labelSafe = true;
    if (!paintOver) 
    {
      if (lit.GetCenterPixel()!=safeLabel)  //check exact voxel in label volume for other labels
        labelSafe = false;
      for (int dim=0; dim<3; dim++) //check all adjacent voxels in a 6 neighborhood in label volume, dimension by dimension for other labels
        if (lit.GetPrevious(dim)!=safeLabel || lit.GetNext(dim)!=safeLabel)
          labelSafe = false;
    }
    
    //Check if voxel is in range 
    if ( (point-initialPoint).GetSquaredNorm()<=centeringRangeSquared && labelSafe) // safe point within recentering search area        
    {
      if (it.Value()>bestUptake)  //Compare to find the best
      {
        centerIndex = it.GetIndex();
        bestUptake = it.Value();
      }
    }
    ++it; ++lit;
  }
  if (bestUptake != itk::NumericTraits<float>::min()) //If no voxel was found safe, use the initial point and give up
    petVolume->TransformIndexToPhysicalPoint(centerIndex, point);
  else  //Otherwise, use the best point
  {
    point[0] = initialPoint[0];
    point[1] = initialPoint[1];
    point[2] = initialPoint[2];
  }
  
  node->SetCenterpoint(point);
  return true;
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::ScalarImageType::Pointer vtkSlicerPETTumorSegmentationLogic::ExtractPETSubVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  if (petVolume.IsNull())
    return NULL;
    
  // identify ROI based on center point, sphere radius, plus a one voxel margin
  ScalarImageType::RegionType roi;
  
  //Find upper and lower points based on the sphereMeshRadius and the centerPoint
  PointType centerPoint = node->GetCenterpoint();
  PointType pointA;
  pointA[0] = centerPoint[0] - meshSphereRadius;
  pointA[1] = centerPoint[1] - meshSphereRadius;
  pointA[2] = centerPoint[2] - meshSphereRadius;
  ScalarImageType::IndexType idxA;
  petVolume->TransformPhysicalPointToIndex(pointA, idxA);  
  PointType pointB;
  pointB[0] = centerPoint[0] + meshSphereRadius;
  pointB[1] = centerPoint[1] + meshSphereRadius;
  pointB[2] = centerPoint[2] + meshSphereRadius;
  ScalarImageType::IndexType idxB;
  petVolume->TransformPhysicalPointToIndex(pointB, idxB);
  
  //Set size and location based on these points
  ScalarImageType::SizeType ROISize;
  ROISize[0] = abs( int(idxA[0])-int(idxB[0]) )+1;
  ROISize[1] = abs( int(idxA[1])-int(idxB[1]) )+1;
  ROISize[2] = abs( int(idxA[2])-int(idxB[2]) )+1;
  ScalarImageType::IndexType ROIStart;
  ROIStart[0] = std::min(idxA[0], idxB[0]);
  ROIStart[1] = std::min(idxA[1], idxB[1]);
  ROIStart[2] = std::min(idxA[2], idxB[2]);
  roi.SetIndex(ROIStart);
  roi.SetSize(ROISize);
  roi.PadByRadius(1); //Margin of error on region
  
  // make sure ROI is fully inside of the given image
  ScalarImageType::RegionType finalROI = petVolume->GetLargestPossibleRegion();
  finalROI.Crop(roi);
  
  // extract subvolume
  typedef itk::RegionOfInterestImageFilter<ScalarImageType, ScalarImageType> ROIExtractorType;
  ROIExtractorType::Pointer roiExtractor = ROIExtractorType::New();
  roiExtractor->SetInput(petVolume);
  roiExtractor->SetRegionOfInterest(finalROI);
  roiExtractor->Update();
  ScalarImageType::Pointer petSubVolume = roiExtractor->GetOutput();
  return petSubVolume;
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::ScalarImageType::Pointer vtkSlicerPETTumorSegmentationLogic::ExtractPETSubVolumeIsotropic(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  ScalarImageType::Pointer petSubVolume = ExtractPETSubVolume(node, petVolume);
  // make subvolume isotropic, choosing the lowest spacing in the original image as the isotropic spacing    
  // calculated target spacing and image size for isotropic image
  // margin of error on the base subvolume is useful here
  // base subvolume also makes resampling much faster
  ScalarImageType::SpacingType spacing = petSubVolume->GetSpacing();
  ScalarImageType::SpacingType origSpacing = petSubVolume->GetSpacing();
  float minSpacing = std::min( std::min(origSpacing[0], origSpacing[1]), origSpacing[2]);
  spacing[0] = minSpacing;
  spacing[1] = minSpacing;
  spacing[2] = minSpacing;
  ScalarImageType::SizeType size;
  ScalarImageType::SizeType origSize = petSubVolume->GetLargestPossibleRegion().GetSize();
  size[0] = (int) std::ceil(origSize[0] * origSpacing[0] / minSpacing);
  size[1] = (int) std::ceil(origSize[1] * origSpacing[1] / minSpacing);
  size[2] = (int) std::ceil(origSize[2] * origSpacing[2] / minSpacing);
  
  // resample image; default interpolation is linear, which is what we want here
  typedef itk::ResampleImageFilter<ScalarImageType, ScalarImageType> ResamplerType;
  ResamplerType::Pointer resampler = ResamplerType::New();
  resampler->SetInput( petSubVolume );
  resampler->SetSize(size);
  resampler->SetOutputSpacing(spacing);
  resampler->SetOutputOrigin(petSubVolume->GetOrigin());
  resampler->SetDefaultPixelValue( 0 );
  
  resampler->Update();
  ScalarImageType::Pointer petSubVolumeIsotropic = resampler->GetOutput();  
  return petSubVolumeIsotropic;
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::GenerateWatershedImages(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petSubVolume)
{
  PointType centerPoint = node->GetCenterpoint();

  //Must create an inverted copy of the image.
  DoubleImageType::Pointer invertedImage = DoubleImageType::New();
  invertedImage->SetRegions(petSubVolume->GetLargestPossibleRegion());
  invertedImage->SetOrigin(petSubVolume->GetOrigin());
  invertedImage->SetSpacing(petSubVolume->GetSpacing());
  invertedImage->Allocate();
  
  itk::ImageRegionIterator<ScalarImageType> datIt(petSubVolume, petSubVolume->GetLargestPossibleRegion());
  itk::ImageRegionIteratorWithIndex<DoubleImageType>  idatIt(invertedImage, invertedImage->GetLargestPossibleRegion());

  datIt.GoToBegin();
  idatIt.GoToBegin();
  //Determine the minimum of the PET volume in the spherical region.
  float radsquared = meshSphereRadius * meshSphereRadius;
  bool minimumUnset = true;
  float regionMinimum = 0;
  while (!datIt.IsAtEnd())
  {
    DoubleImageType::IndexType curIndex = idatIt.GetIndex();
    DoubleImageType::PointType curPoint;
    invertedImage->TransformIndexToPhysicalPoint(curIndex, curPoint);
    float dist =  (curPoint[0]-centerPoint[0])*(curPoint[0]-centerPoint[0]) +
                  (curPoint[1]-centerPoint[1])*(curPoint[1]-centerPoint[1]) +
                  (curPoint[2]-centerPoint[2])*(curPoint[2]-centerPoint[2]);
    if (dist <= radsquared && (datIt.Get() < regionMinimum || minimumUnset == true))
    {
      regionMinimum = datIt.Get();
      minimumUnset = false;
    }
    ++datIt;
    ++idatIt;
  }
  datIt.GoToBegin();
  idatIt.GoToBegin();
  //Set all voxels outside the spherical region to the inverse of that minimum, and the rest to the inverse of the PET data.
  //This ensures that the level used by the watersheds is purely based on the spherical region of interest.
  while (!datIt.IsAtEnd())
  {
    DoubleImageType::IndexType curIndex = idatIt.GetIndex();
    DoubleImageType::PointType curPoint;
    invertedImage->TransformIndexToPhysicalPoint(curIndex, curPoint);
    float dist =  (curPoint[0]-centerPoint[0])*(curPoint[0]-centerPoint[0]) +
                  (curPoint[1]-centerPoint[1])*(curPoint[1]-centerPoint[1]) +
                  (curPoint[2]-centerPoint[2])*(curPoint[2]-centerPoint[2]);
    if (dist <= radsquared)
    { idatIt.Set(-1 * datIt.Get()); }
    else
    { idatIt.Set(-1 * regionMinimum);  }
    
    ++datIt;
    ++idatIt;
  }
  
  //Generate each watershed and store it locally.  This reduces long term storage while preventing too much recalculation of watershed volumes.
  typedef itk::WatershedImageFilter<DoubleImageType> WatershedImageFilterType;
  WatershedImageFilterType::Pointer strongWatershedFilter = WatershedImageFilterType::New();
  strongWatershedFilter->SetInput(invertedImage);
  strongWatershedFilter->SetLevel(0.20); //Level is the peak to barrier difference.  Higher level is more likely to reject more walls
  strongWatershedFilter->SetThreshold(0.00); //Also helps reject walls
  strongWatershedFilter->Update();
  WatershedImageType::Pointer strongWatershedImage = strongWatershedFilter->GetOutput();
  strongWatershedImage->DisconnectPipeline();
  StrongWatershedVolume_saved = strongWatershedImage;
  
  WatershedImageFilterType::Pointer weakWatershedFilter = WatershedImageFilterType::New();
  weakWatershedFilter->SetInput(invertedImage);
  weakWatershedFilter->SetLevel(0.00); //Level is the peak to barrier difference.  Higher level is more likely to reject more walls
  weakWatershedFilter->SetThreshold(0.00); //Also helps reject walls
  weakWatershedFilter->Update();
  WatershedImageType::Pointer weakWatershedImage = weakWatershedFilter->GetOutput() ;
  weakWatershedImage->DisconnectPipeline();
  WeakWatershedVolume_saved = weakWatershedImage;

}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::CreateGraph(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  PointType centerpoint = node->GetCenterpoint();
  
  // create graph structure using spherical mesh as initial surface.
  
  // create a spherical mesh to base the graph off of
  typedef itk::RegularSphereMeshSource<MeshType> RegularSphereMeshSourceType;
  RegularSphereMeshSourceType::Pointer sphereMeshSource = RegularSphereMeshSourceType::New();
  RegularSphereMeshSourceType::PointType sphereCenter;
  sphereCenter[0] = centerpoint[0];  sphereCenter[1] = centerpoint[1];  sphereCenter[2] = centerpoint[2];
  RegularSphereMeshSourceType::VectorType sphereRadius;
  sphereRadius.Fill(meshSphereRadius);
  sphereMeshSource->SetCenter( sphereCenter );
  sphereMeshSource->SetScale( sphereRadius );
  sphereMeshSource->SetResolution( meshResolution );

  // create OSF graph surface from mesh
  typedef itk::MeshToOSFGraphFilter<MeshType, OSFGraphType> MeshToOSFGraphFilterType;
  MeshToOSFGraphFilterType::Pointer meshToOSFGraphFilter = MeshToOSFGraphFilterType::New();
  meshToOSFGraphFilter->SetInput( sphereMeshSource->GetOutput() );
  meshToOSFGraphFilter->Update();
  
  node->SetOSFGraph( meshToOSFGraphFilter->GetOutput() );
  // create columns from the center to the vertices of the mesh
  int numVertices = node->GetOSFGraph()->GetSurface()->GetNumberOfVertices();
  itk::Workers().RunFunctionForRange<int, vtkMRMLPETTumorSegmentationParametersNode*>
    (&BuildColumnForVertex, 0, numVertices-1, node);  
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::BuildColumnForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node)
{
  PointType centerpoint = node->GetCenterpoint();
  
  OSFSurfaceType::Pointer surface =  node->GetOSFGraph()->GetSurface();  
  typedef OSFSurfaceType::CoordinateType Coordinate;
  typedef OSFSurfaceType::ColumnCoordinatesContainer ColumnCoordinatesContainer;

  //Create the new container of appropriate size
  int numberOfSteps = (int) std::ceil(meshSphereRadius); //TODO Should this be meshSphereRadius/columnStepSize? As it is, it assumes columnStepSize==1, which for now it does, but still might be worth modifying for future use.
  ColumnCoordinatesContainer::Pointer columnPositions = ColumnCoordinatesContainer::New();
  columnPositions->CreateIndex( numberOfSteps-1 );  
  
  // build columns along straight line from center outwards
  PointType initialVertexPosition = surface->GetInitialVertexPosition( vertexId );
  PointType::VectorType direction = initialVertexPosition-centerpoint;
  direction.Normalize();
  for (int step=0; step<numberOfSteps; step++)  //Place each point based on the center point, the direction, and the number and size of the steps
  {
    Coordinate currentPosition = centerpoint + direction*columnStepSize*float(step+1);
    columnPositions->SetElement( step, currentPosition );
  }

  surface->SetColumnCoordinates( vertexId, columnPositions );
  surface->GetColumnCosts( vertexId )->CreateIndex( columnPositions->Size()-1 );
  surface->SetInitialVertexPositionIdentifier( vertexId, 0 );
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::ObtainHistogram(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  PointType centerPoint = node->GetCenterpoint();
  ScalarImageType::Pointer petSubVolumeIsotropic = ExtractPETSubVolumeIsotropic(node, petVolume);
  if (petSubVolumeIsotropic.IsNull())
    return;
  
  // create list of uptakes of all voxels inside of sphere
  std::vector<float> pixelData;
  itk::ImageRegionIterator<ScalarImageType> it(petSubVolumeIsotropic, petSubVolumeIsotropic->GetLargestPossibleRegion());
  float meshSphereRadiusSquared = meshSphereRadius*meshSphereRadius;
  while (!it.IsAtEnd())
  { 
    ScalarImageType::PointType currentPoint;
    petSubVolumeIsotropic->TransformIndexToPhysicalPoint(it.GetIndex(), currentPoint);
    float distance_squared = (currentPoint-centerPoint).GetSquaredNorm(); //use squared distance to avoid long sqrt calculations
    if (distance_squared<=meshSphereRadiusSquared)
      pixelData.push_back(it.Get());
    ++it;
  }
  
  // obtain min, max and median values
  std::sort(pixelData.begin(), pixelData.end());
  //float minValue = pixelData[0];  min is never used
  float medianValue = pixelData[pixelData.size()/2];
  float maxValue = pixelData[pixelData.size()-1];
  
  // build histogram
  std::vector<float> histogram(numHistogramBins,0);
  for (size_t i=0; i < pixelData.size(); ++i)
  {
    int index = (int) ((pixelData[i] / maxValue) * numHistogramBins);
    index = std::max( std::min(index, int(numHistogramBins)-1), 0);
    histogram[index]++;
  }
  
  // make sure histogram value never falls (envelope function)
  for (int i=histogram.size()-2; i>=0; --i)
    histogram[i] = std::max(histogram[i],histogram[i+1]);
  
  // normalize histogram to range 0.0 to 1.0
  float normalizationFactor = histogram[0];
  for (size_t i=0; i<histogram.size(); ++i)
    histogram[i] /= normalizationFactor;
    
  node->SetHistogram(histogram);
  node->SetHistogramRange(maxValue);
  node->SetHistogramMedian(medianValue);
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::MaxFlow(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if (graph.IsNull())
    return;
  
  OSFSurfaceType::Pointer surface = node->GetOSFGraph()->GetSurface();
  // add smoothness constraints to graph
  typedef itk::SimpleOSFGraphBuilderFilter<OSFGraphType,OSFGraphType> GraphBuilderType;
  GraphBuilderType::Pointer graphBuilder = GraphBuilderType::New();
  graphBuilder->SetInput( graph );
  graphBuilder->SetSmoothnessConstraint( hardSmoothnessConstraint );
  graphBuilder->SetSoftSmoothnessPenalty( node->GetSplitting() ? softSmoothnessPenaltySplitting : softSmoothnessPenalty );

  // run the max flow algorithm to solve the segmentation problem
  typedef itk::LOGISMOSOSFGraphSolverFilter<OSFGraphType,OSFGraphType> OSFGraphSolverType;
  OSFGraphSolverType::Pointer osfGraphSolver = OSFGraphSolverType::New();
  osfGraphSolver->SetInput( graphBuilder->GetOutput() );
  osfGraphSolver->Update();
  
  OSFGraphType::Pointer solvedGraph = osfGraphSolver->GetOutput();
  node->SetOSFGraph( solvedGraph );
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::MeshType::Pointer vtkSlicerPETTumorSegmentationLogic::GetSegmentationMesh(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  // extract the segmentation surface from the OSF graph as a mesh
  OSFGraphType::Pointer solvedGraph = node->GetOSFGraph();
  if (solvedGraph.IsNull())
    return NULL;
    
  // Get resulting surface mesh from osf graph
  typedef itk::OSFGraphToMeshFilter<OSFGraphType,MeshType> OSFGraphToMeshFilterType;
  OSFGraphToMeshFilterType::Pointer osfGraphToMeshFilter = OSFGraphToMeshFilterType::New();
  osfGraphToMeshFilter->SetInput( solvedGraph );
  osfGraphToMeshFilter->Update();
  MeshType::Pointer segmentationMesh = osfGraphToMeshFilter->GetOutput();
  
  return segmentationMesh;
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::CalculateThresholdHistogramBased(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{  
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if (graph.IsNull() || petVolume.IsNull())
    return;
  
  // denoise hisogram if requested
  ScalarImageType::Pointer medianPetVolume = NULL;
  if (node->GetDenoiseThreshold())  //set the medianPetVolume only if needed
  {
    typedef itk::MedianImageFilter<ScalarImageType, ScalarImageType> MedianFilterType;
    MedianFilterType::Pointer medianFilter = MedianFilterType::New();
    medianFilter->SetInput(ExtractPETSubVolume(node, petVolume));

    ScalarImageType::SizeType indexRadius;
    indexRadius[0] = 1;
    indexRadius[1] = 1;
    indexRadius[2] = 1;
    medianFilter->SetRadius( indexRadius );
    medianFilter->Update();
    medianPetVolume = medianFilter->GetOutput();
    medianPetVolume->DisconnectPipeline();
  }

  // obtain shell uptake values
  int numberOfShells = graph->GetSurface()->GetNumberOfColumns(0);
  std::vector<float> shellUptake(numberOfShells, 0.0);
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  if (node->GetDenoiseThreshold())  //determine shells on median pet volume only if needed
    interpolator->SetInputImage( medianPetVolume );
  else
    interpolator->SetInputImage( petVolume );
  itk::Workers().RunFunctionForRange<int, vtkMRMLPETTumorSegmentationParametersNode*, std::vector<float>&, InterpolatorType::Pointer>
    (&GetMedianUptakeForShell, 0, numberOfShells-1, node, shellUptake, interpolator);

  

  // find peak and knee values
  float peakValue = *(max_element(shellUptake.begin(), shellUptake.end()));
  std::vector<float> gradients(numberOfShells, 0.0);
  std::vector<float> biasedGradients(numberOfShells, 0.0);
  
  // make gradients and biased gradients
  for (int i=1; i<numberOfShells-1; i++)
  {
    gradients[i] = shellUptake[i+1] - shellUptake[i-1];
    biasedGradients[i] = gradients[i]*(float(numberOfShells) - (1.0+i))/float(numberOfShells);
  }
  
  // find low side of biased gradient as the index
  int gradient_low_index = std::min_element(biasedGradients.begin()+1, biasedGradients.end()-1)-biasedGradients.begin();
  float gradient_low = gradients[gradient_low_index];
  std::vector<float> gradientsRising = gradients;
  
  //generate envelope function above the gradient low index
  float current_max = gradients[gradient_low_index];
  for (int i=gradient_low_index; i<numberOfShells-1; i++)
  {
    current_max = std::max(gradients[i], current_max);
    gradientsRising[i] = current_max;
  }
  
  //calculate the gradient target value for the knee
  float gradient_rising_high = std::min( 0.0f, *(max_element(gradientsRising.begin()+gradient_low_index, gradientsRising.end()-1)) );
  float gradient_rising_knee = 0.75*gradient_rising_high + 0.25*gradient_low;

  //finding lowest absolute difference
  //intelligent tiebreaker: the closest will be on one side or the other of when the differences passes 0
  //                        on that pass, pick the side that is closer to 0
  //                        if the closest numerically is a stretch of equal values, this makes sure the one closest on the curve to the 0 crossing is selected
  //                        otherwise, the same result as a numerical closest is found
  int knee_index = 0;
  bool foundTransition = false;
  int seekKnee = gradient_low_index;
  //search specifically for the transition from too low to too high
  while (foundTransition == false && seekKnee < numberOfShells-1)
  {
    float curDif = gradientsRising[seekKnee]-gradient_rising_knee;
    //if exact point is found, take it
    if (curDif == 0)
    {
      knee_index = seekKnee;
      foundTransition = true;
    } //check current and next difference; if sign change, then we've found the transition
    else if (curDif < 0 && gradientsRising[seekKnee+1]-gradient_rising_knee > 0)
    {
      
      if (std::abs(curDif) <= gradientsRising[seekKnee+1]-gradient_rising_knee)
        knee_index = seekKnee;  //if before transition is closer, use that
      else
        knee_index = seekKnee+1;  //if after transition is closer, use that
      foundTransition = true; //regardless, stop searching after finding the transition
    }
    seekKnee++;
  } 

  float kneeValue = shellUptake[knee_index];  
  
  // calculate threshold
  float coefficient = kneeValue/peakValue;
  float threshold_percentage = 0.8 * exp(-0.15/(sqrt(coefficient)*coefficient));
  float threshold = kneeValue + threshold_percentage*(peakValue-kneeValue);
  node->SetThreshold(threshold);
  
  
  // obtain sphere center uptake value, always from normal PET volume
  float centerpointUptake = 0.0;
  if (!node->GetDenoiseThreshold())
  {
    if (interpolator->IsInsideBuffer( node->GetCenterpoint() ) )
      centerpointUptake = interpolator->Evaluate( node->GetCenterpoint() );
  }
  else  //in case of denoise threshold, normal interpolator is otherwise occupied, so use a different one quick
  {
    InterpolatorType::Pointer tempInterpolator = InterpolatorType::New();
    tempInterpolator->SetInputImage( petVolume );
    if (tempInterpolator->IsInsideBuffer( node->GetCenterpoint() ) )
      centerpointUptake = tempInterpolator->Evaluate( node->GetCenterpoint() );
  }
  node->SetCenterpointUptake(centerpointUptake);
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::GetMedianUptakeForShell(int shellId, vtkMRMLPETTumorSegmentationParametersNode* node, std::vector<float>& shellUptakes, InterpolatorType::Pointer interpolator)
{
  OSFSurfaceType::Pointer surface = node->GetOSFGraph()->GetSurface();
  unsigned int numberOfVertices = surface->GetNumberOfVertices();
  std::vector<float> shellValues(numberOfVertices, 0.0);
  
  // a shell is all nodes a certain distance from the center
  // median must be taken of uptake at all nodes at the distance
  for (unsigned int i=0; i<numberOfVertices; ++i)
  {
    const OSFSurfaceType::CoordinateType& coordinate = surface->GetColumnCoordinates(i)->ElementAt(shellId);
    float uptake = 0;
    if ( interpolator->IsInsideBuffer( coordinate ) )
      uptake = interpolator->Evaluate( coordinate );   
    shellValues[i] = uptake;
  }
  
  //sort and take middle index to get median
  std::sort(shellValues.begin(), shellValues.end());
  shellUptakes[shellId] = shellValues[shellValues.size()/2];
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::CalculateThresholdPointLocationBased(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  vtkMRMLFiducialListNode* globalRefinementFiducials = static_cast<vtkMRMLFiducialListNode*>( node->GetScene()->GetNodeByID( node->GetGlobalRefinementIndicatorListReference()) );
  if (globalRefinementFiducials->GetNumberOfFiducials()==0 || petVolume.IsNull() )
    return;
  
  // utilize global refinement fiducial to determine threshold
  PointType refinementPoint = convert2ITK( globalRefinementFiducials->GetNthFiducialXYZ(globalRefinementFiducials->GetNumberOfFiducials()-1) );
  IndexType index;
  petVolume->TransformPhysicalPointToIndex(refinementPoint, index);
    

  // check that point is within region of pet and label volumes, and not right at the edges, due to problems that causes.
  ScalarImageType::RegionType petRegion = petVolume->GetLargestPossibleRegion();
  ScalarImageType::IndexType petLowestIndex = petRegion.GetIndex();
  ScalarImageType::IndexType petHighestIndex = petRegion.GetIndex();
  petHighestIndex[0] += petRegion.GetSize()[0]-1;
  petHighestIndex[1] += petRegion.GetSize()[1]-1;
  petHighestIndex[2] += petRegion.GetSize()[2]-1;
  
  bool inside = true;
  for (int dim = 0; dim < 3 && inside == true; dim++) //check bounds on each dimension
  {
    if (index[dim] < petLowestIndex[dim] || index[dim] > petHighestIndex[dim])
    { inside = false; }
  }
  
  if (inside) //if successful, set threshold to the exact uptake at the nearest voxel
  {
    float threshold = petVolume->GetPixel(index);
    node->SetThreshold(threshold);
  }
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::LabelImageType::Pointer vtkSlicerPETTumorSegmentationLogic::GetSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, MeshType::Pointer segmentationMesh, LabelImageType::Pointer initialLabelMap)
{
  if (segmentationMesh.IsNull() || initialLabelMap.IsNull())
    return NULL;

  // voxelize mesh
  typedef itk::TriangleMeshToBinaryImageFilter<MeshType, LabelImageType> MeshToLabelImageFilterType;
  MeshToLabelImageFilterType::Pointer meshToImage = MeshToLabelImageFilterType::New();
  meshToImage->SetInsideValue(1);
  meshToImage->SetOutsideValue(0);
  meshToImage->SetInput(segmentationMesh);
  //meshToImage->SetInfoImage( initialLabelMap ); // this outputs information to the console; setting image info manually as done below does not output to the console
  meshToImage->SetSpacing(initialLabelMap->GetSpacing());
  meshToImage->SetOrigin(initialLabelMap->GetOrigin());
  meshToImage->SetSize(initialLabelMap->GetLargestPossibleRegion().GetSize());
  meshToImage->SetIndex(initialLabelMap->GetLargestPossibleRegion().GetIndex());
  meshToImage->Update();
  LabelImageType::Pointer segmentation = meshToImage->GetOutput();
  
  if (node->GetPaintOver() == false)
  { //remove voxels in other lesions
    short label = node->GetLabel();
    typedef itk::ImageRegionIterator<LabelImageType> IteratorType;
    IteratorType labelIt(initialLabelMap, initialLabelMap->GetLargestPossibleRegion());
    IteratorType newIt(segmentation, segmentation->GetLargestPossibleRegion());
    while (!(labelIt.IsAtEnd() || newIt.IsAtEnd()))
    {
      if (labelIt.Get() != 0 && labelIt.Get() != label)
      { newIt.Set(0); }
      ++newIt;
      ++labelIt;
    }
  }
  
  // do 6-connected region growing to remove unconnected voxels that may result from the voxelization process
  typedef itk::ConnectedThresholdImageFilter< LabelImageType, LabelImageType > ConnectedComponentFilterType;
  ConnectedComponentFilterType::Pointer connectedComponentFilter = ConnectedComponentFilterType::New();
  connectedComponentFilter->SetInput( segmentation );
  connectedComponentFilter->SetConnectivity(ConnectedComponentFilterType::FaceConnectivity);
  ConnectedComponentFilterType::IndexType seed;
  segmentation->TransformPhysicalPointToIndex( node->GetCenterpoint(), seed);
  connectedComponentFilter->AddSeed(seed);
  connectedComponentFilter->SetUpper(1);
  connectedComponentFilter->SetLower(1);
  connectedComponentFilter->Update();
  segmentation = connectedComponentFilter->GetOutput();
  return segmentation;
}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::UpdateOutput(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer segmentation, LabelImageType::Pointer initialLabelMap)
{
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  
  short label = node->GetLabel();
  bool paintOver = node->GetPaintOver();
  bool sealing = node->GetSealing();
  bool necroticRegion = node->GetNecroticRegion();
  float threshold = node->GetThreshold();
  if (segmentation.IsNull() || initialLabelMap.IsNull() || petVolume.IsNull())
    return;
  // merge segmentation and seal if requested
  typedef itk::SealingSegmentationMergerImageFilter<LabelImageType, ScalarImageType, LabelImageType> SegmentationMergerType;
  SegmentationMergerType::Pointer segmentationMerger = SegmentationMergerType::New();
  segmentationMerger->SetInput(segmentation);
  segmentationMerger->SetLabelImage(initialLabelMap);
  segmentationMerger->SetDataImage(petVolume);
  segmentationMerger->SetThreshold(threshold); // TODO: set threshold to 0.0 in case necrotic tumor option is turned on
  segmentationMerger->SetLabel(label);
  segmentationMerger->SetPaintOver(paintOver);
  segmentationMerger->SetSealing(sealing);
  segmentationMerger->SetNecroticRegion(necroticRegion);
  segmentationMerger->Update();
  LabelImageType::Pointer labelMap = segmentationMerger->GetOutput();
  
  vtkMRMLScalarVolumeNode* segmentationVolumeNode = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationVolumeReference() ));
  if (segmentationVolumeNode!=NULL) // for use with segmentation editor (vtkLabelMap)
  {
    // convert itk label map to vtk label map and update label volume node in MRML scene
    vtkSmartPointer<vtkImageData> vtkLabelMap = convert2VTK<LabelImageType>(labelMap, VTK_SHORT);
    // copy image data directly into existing volume, avoiding any chance for accidental memory leaks
    segmentationVolumeNode->GetImageData()->DeepCopy( vtkLabelMap );
    segmentationVolumeNode->Modified();
  }
  else // for use with segment editor (vtkSegmentation)
  {
    vtkMRMLSegmentationNode* vtkSegmentationNode = static_cast<vtkMRMLSegmentationNode*>(slicerMrmlScene->GetNodeByID( node->GetSegmentationReference() ));
    vtkMRMLScalarVolumeNode* vtkPetVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetPETVolumeReference() ));
    vtkSmartPointer<vtkOrientedImageData> referenceGeometry = 
      vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(vtkPetVolume); // todo: this seems to be expensive just to get the orientation information
      
    // iterate over all segments
    std::vector< std::string > segmentIds;
    vtkSegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIds);
    for (size_t i=0; i<segmentIds.size(); i++)
    { 
      if (!node->GetPaintOver() && segmentIds[i]!=node->GetSelectedSegmentID())
        continue; // this segment doesn't need an update      
      short label = i+1;  
    
      // get binary image of current segment
      typedef itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType> BinaryThresholdImageFilterType;
      BinaryThresholdImageFilterType::Pointer binaryThresholdFilter = BinaryThresholdImageFilterType::New();
      binaryThresholdFilter->SetInput( labelMap );
      binaryThresholdFilter->SetLowerThreshold( label );
      binaryThresholdFilter->SetUpperThreshold( label );
      binaryThresholdFilter->SetInsideValue(1);
      binaryThresholdFilter->SetOutsideValue(0);
      binaryThresholdFilter->Update();
      LabelImageType::Pointer segmentLabelMap = binaryThresholdFilter->GetOutput();
    
      // convert to vtk
      vtkSmartPointer<vtkOrientedImageData> vtkLabelVolume = vtkSmartPointer<vtkOrientedImageData>::New();
      vtkSmartPointer<vtkImageData> vtkLabelMap = convert2VTK<LabelImageType>(segmentLabelMap, VTK_SHORT);
      vtkLabelMap->SetSpacing(segmentLabelMap->GetSpacing()[0], labelMap->GetSpacing()[1], segmentLabelMap->GetSpacing()[2]);
      vtkLabelMap->SetOrigin(-segmentLabelMap->GetOrigin()[0], -labelMap->GetOrigin()[1], segmentLabelMap->GetOrigin()[2]);
      vtkLabelVolume->ShallowCopy(vtkLabelMap);vtkLabelVolume->CopyDirections(referenceGeometry);
    
      // update segment in segmentation
      vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(vtkLabelVolume, vtkSegmentationNode, segmentIds[i], vtkSlicerSegmentationsModuleLogic::MODE_REPLACE);
    }
    
    referenceGeometry->Delete();
  }

}

//----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // TODO: implement
}

//---------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    {
    return;
    }
  vtkMRMLPETTumorSegmentationParametersNode* pNode = vtkMRMLPETTumorSegmentationParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(pNode);
  pNode->Delete();
}

//---------------------------------------------------------------------------
// TODO: do we need to implement this, or can we use the superclasse's implementation?
void vtkSlicerPETTumorSegmentationLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
// TODO: do we need to implement this, or can we use the superclasse's implementation?
void vtkSlicerPETTumorSegmentationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
// TODO: do we need to implement this, or can we use the superclasse's implementation?
void vtkSlicerPETTumorSegmentationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
// todo: move into separate conversion utils file
template <class ITKImageType>
typename ITKImageType::Pointer
vtkSlicerPETTumorSegmentationLogic::convert2ITK(vtkSmartPointer<vtkImageData> vtkVolume)
{
  // create ITK image with empty pixel buffer
  typename ITKImageType::Pointer itkVolume = ITKImageType::New();
  typename ITKImageType::RegionType region;
  region.SetSize(0, vtkVolume->GetDimensions()[0]);
  region.SetSize(1, vtkVolume->GetDimensions()[1]);
  region.SetSize(2, vtkVolume->GetDimensions()[2]);
  itkVolume->SetRegions( region );
  itkVolume->Allocate();
  itkVolume->SetSpacing( vtkVolume->GetSpacing() );
  double origin[3] = {-itkVolume->GetOrigin()[0], -itkVolume->GetOrigin()[1], itkVolume->GetOrigin()[2]}; // FIXXME: I think this should be vtkVolume instead!
  itkVolume->SetOrigin( origin );
  unsigned long numVoxels = itkVolume->GetLargestPossibleRegion().GetNumberOfPixels();
  
  if( vtkVolume->GetScalarType() == vtkTypeTraits<typename ITKImageType::PixelType>::VTKTypeID() ) // correct datatype
  {
    memcpy( itkVolume->GetBufferPointer(), vtkVolume->GetScalarPointer(), numVoxels*sizeof(typename ITKImageType::PixelType) );
  }
  else // cast to correct data type needed
  {
    vtkSmartPointer<vtkImageCast> cast = vtkImageCast::New();
    cast->SetInputData(vtkVolume);
    cast->SetOutputScalarType( vtkTypeTraits<typename ITKImageType::PixelType>::VTKTypeID() );
    cast->Update();
    memcpy( itkVolume->GetBufferPointer(), cast->GetOutput()->GetScalarPointer(), numVoxels*sizeof(typename ITKImageType::PixelType) );
    cast->Delete();
  }
  
  return itkVolume;
}

//---------------------------------------------------------------------------
// TODO: move into separate conversion utils file
template <class ITKImageType>
vtkSmartPointer<vtkImageData> vtkSlicerPETTumorSegmentationLogic::convert2VTK(typename ITKImageType::Pointer itkVolume, const int TypeVTK)
{  
  vtkSmartPointer<vtkImageData> vtkImage = vtkSmartPointer<vtkImageData>::New();
  vtkImage->SetDimensions( itkVolume->GetLargestPossibleRegion().GetSize()[0],
                           itkVolume->GetLargestPossibleRegion().GetSize()[1],
                           itkVolume->GetLargestPossibleRegion().GetSize()[2] );
  vtkImage->AllocateScalars( vtkTypeTraits<typename ITKImageType::PixelType>::VTKTypeID(), 1);
  //Figure out the exact size of the data to copy and copy it directly from the vtk data pointer to the itk data pointer
  
  unsigned long numVoxels = itkVolume->GetLargestPossibleRegion().GetNumberOfPixels();
  memcpy( vtkImage->GetScalarPointer(), itkVolume->GetBufferPointer(), numVoxels*sizeof(typename ITKImageType::PixelType) );

  if(vtkImage->GetScalarType() != TypeVTK)
  {
    vtkSmartPointer<vtkImageCast> cast = vtkImageCast::New();
    cast->SetInputData(vtkImage);
    cast->SetOutputScalarType(TypeVTK);
    cast->Update();
    vtkImage = cast->GetOutput();
    cast->Delete();
  }
  
  // TODO: do we have to copy any other information from the image header?
  return vtkImage;
}

//---------------------------------------------------------------------------
// TODO: move into separate conversion utils file
vtkSlicerPETTumorSegmentationLogic::PointType
vtkSlicerPETTumorSegmentationLogic::convert2ITK(const float* coordinate)
{
  PointType p;
  p[0] = -coordinate[0];
  p[1] = -coordinate[1];
  p[2] = coordinate[2];
  
  return p;  
}

//---------------------------------------------------------------------------
// TODO: move into separate conversion utils file
template <class ITKImageType>
void vtkSlicerPETTumorSegmentationLogic::writeImage(typename ITKImageType::Pointer itkVolume, char* filename)
{
  typedef itk::ImageFileWriter<ITKImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetInput( itkVolume );
  writer->SetFileName( filename );
  writer->Update();
}

//---------------------------------------------------------------------------
// TODO: move into separate conversion utils file
template <class ITKMeshType>
void vtkSlicerPETTumorSegmentationLogic::writeMesh(typename ITKMeshType::Pointer itkMesh, char* filename)
{
  typedef itk::MeshFileWriter<ITKMeshType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetInput( itkMesh );
  writer->SetFileName( filename );
  writer->Update();
}


//---------------------------------------------------------------------------
// TODO: move into separate conversion utils file
template <class ITKImageType, class ITKImage2Type>
typename ITKImageType::Pointer vtkSlicerPETTumorSegmentationLogic::resampleNN(typename ITKImageType::Pointer image, typename ITKImage2Type::Pointer targetImage)
{
  typedef itk::ResampleImageFilter< ITKImageType, ITKImageType > ResampleFilterType;
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( image );
  resampler->SetOutputParametersFromImage( targetImage );
  resampler->SetInterpolator( itk::NearestNeighborInterpolateImageFunction< ITKImageType, double >::New() );
  resampler->SetDefaultPixelValue( 0 );
  resampler->Update();
  return resampler->GetOutput();
}

//---------------------------------------------------------------------------
int vtkSlicerPETTumorSegmentationLogic::GetClosestVertex(vtkMRMLPETTumorSegmentationParametersNode* node, const PointType& p)
{
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if (graph.IsNull())
    return 0;
    
  // note: this implementation assumes that the mesh is spherical with straight columns pointing away from the center of the sphere
  // therefore, we only need to find the closest point on a shell and don't have to search the whole columns
  OSFSurfaceType::Pointer surface = node->GetOSFGraph()->GetSurface();
  int numVertices = surface->GetNumberOfVertices();
  std::vector<float> distancesSquared(numVertices,0.0);
  for (int i=0; i<numVertices; ++i)
    distancesSquared[i] = (surface->GetColumnCoordinates(i)->GetElement(0) - p).GetSquaredNorm();    
  return std::min_element(distancesSquared.begin(), distancesSquared.end()) - distancesSquared.begin();
}

//---------------------------------------------------------------------------
int vtkSlicerPETTumorSegmentationLogic::GetClosestColumnOnVertex(vtkMRMLPETTumorSegmentationParametersNode* node, const PointType& p, int vertexId)
{
  OSFGraphType::Pointer graph = node->GetOSFGraph();
  if (graph.IsNull())
    return 0;
  
  // with the vertex already determined, this is easy
  OSFSurfaceType::ColumnCoordinatesContainer::ConstPointer columnPoints = node->GetOSFGraph()->GetSurface()->GetColumnCoordinates(vertexId);
  int numPoints = columnPoints->Size();
  std::vector<float> distancesSquared(numPoints,0.0);
  for (int i=0; i<numPoints; ++i)
    distancesSquared[i] = (columnPoints->GetElement(i) - p).GetSquaredNorm();
  return std::min_element(distancesSquared.begin(), distancesSquared.end()) - distancesSquared.begin();
}


//---------------------------------------------------------------------------
void vtkSlicerPETTumorSegmentationLogic::UpdateFingerPrint(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  // finger prints are used to track need for recalculating the watersheds
  // new watersheds are only needed if either the PET volume or center point change, so save knowledge of those
  std::string volumeFingerPrint_node = node->GetPETVolumeReference();
  if (volumeFingerPrint.compare(volumeFingerPrint_node) != 0) //new PET volume
  {
    centerFingerPrint.clear();
    volumeFingerPrint = volumeFingerPrint_node;
  }
  vtkMRMLFiducialListNode* centerFiducials = static_cast<vtkMRMLFiducialListNode*>(node->GetScene()->GetNodeByID( node->GetCenterPointIndicatorListReference() ));
  if (centerFiducials->GetNumberOfFiducials()==0) //if no center, clear everything
  {
    StrongWatershedVolume_saved = NULL;
    WeakWatershedVolume_saved = NULL;
    centerFingerPrint.clear();
    return;
  }
  PointType centerFingerPrint_node = convert2ITK( centerFiducials->GetNthFiducialXYZ(centerFiducials->GetNumberOfFiducials()-1) );
  if (centerFingerPrint.size() == 0 || centerFingerPrint_node[0] != centerFingerPrint[0] || centerFingerPrint_node[1] != centerFingerPrint[1] || centerFingerPrint_node[2] != centerFingerPrint[2]) //set the center point if it doesn't already match
  {
    StrongWatershedVolume_saved = NULL;
    WeakWatershedVolume_saved = NULL;
    centerFingerPrint.resize(3);
    centerFingerPrint[0] = centerFingerPrint_node[0];
    centerFingerPrint[1] = centerFingerPrint_node[1];
    centerFingerPrint[2] = centerFingerPrint_node[2];
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerPETTumorSegmentationLogic::CheckFingerPrint(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  if (!node)
    return false;
  std::string volumeFingerPrint_node = node->GetPETVolumeReference();
  if (volumeFingerPrint.compare(volumeFingerPrint_node) == 0) //volume is most major change; different volumes could have same center point for segmentation
  {
    if (centerFingerPrint.size() == 0)  //no center means no match
      return false;
    vtkMRMLFiducialListNode* centerFiducials = static_cast<vtkMRMLFiducialListNode*>(node->GetScene()->GetNodeByID( node->GetCenterPointIndicatorListReference() ));
    if (centerFiducials->GetNumberOfFiducials() == 0)  //no center means no match
      return false;
    PointType centerFingerPrint_node = convert2ITK( centerFiducials->GetNthFiducialXYZ(centerFiducials->GetNumberOfFiducials()-1) );
    return (centerFingerPrint_node[0] == centerFingerPrint[0] && centerFingerPrint_node[1] == centerFingerPrint[1] && centerFingerPrint_node[2] == centerFingerPrint[2]);
  }
  else
  { return false; }
}


//---------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::ScalarImageType::Pointer vtkSlicerPETTumorSegmentationLogic::GetPETVolume(vtkMRMLPETTumorSegmentationParametersNode* node)
{
  //Converts the PET volume from reference to vtk volume to itk volume
  ScalarImageType::Pointer petVolume = NULL;
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  vtkMRMLScalarVolumeNode* vtkPetVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( node->GetPETVolumeReference() ));
  petVolume = convert2ITK<ScalarImageType>( vtkPetVolume->GetImageData() );
  petVolume->SetSpacing( vtkPetVolume->GetSpacing() );
  double origin2[3] = {-vtkPetVolume->GetOrigin()[0], -vtkPetVolume->GetOrigin()[1], vtkPetVolume->GetOrigin()[2]};
  petVolume->SetOrigin( origin2 );
  
  return petVolume;
}

//---------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::WatershedImageType::Pointer vtkSlicerPETTumorSegmentationLogic::GetStrongWatershedVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  //If the finger prints match, return the existing saved strong watershed.
  //Otherwise, create both watershed images.
  WatershedImageType::Pointer strongWatershedVolume;
  bool fingerprint = CheckFingerPrint(node);
  if (fingerprint && !StrongWatershedVolume_saved.IsNull()) //Finger print matches
  { strongWatershedVolume = StrongWatershedVolume_saved;  }
  else  //Finger print does not match
  {
    if (!fingerprint)
    { UpdateFingerPrint(node);  }
    GenerateWatershedImages(node, ExtractPETSubVolume(node, petVolume));
    strongWatershedVolume = StrongWatershedVolume_saved;
  }
  return strongWatershedVolume;
}

//---------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::WatershedImageType::Pointer vtkSlicerPETTumorSegmentationLogic::GetWeakWatershedVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume)
{
  //If the finger prints match, return the existing saved weak watershed.
  //Otherwise, create both watershed images.
  WatershedImageType::Pointer weakWatershedVolume;
  bool fingerprint = CheckFingerPrint(node);
  if (fingerprint && !WeakWatershedVolume_saved.IsNull()) //Finger print matches
  { weakWatershedVolume = WeakWatershedVolume_saved;  }
  else  //Finger print does not match
  {
    if (!fingerprint)
    { UpdateFingerPrint(node);  }
    GenerateWatershedImages(node, ExtractPETSubVolume(node, petVolume));
    weakWatershedVolume = WeakWatershedVolume_saved;
  }
  return weakWatershedVolume;
}

//----------------------------------------------------------------------------
vtkSlicerPETTumorSegmentationLogic::OSFGraphType::Pointer 
vtkSlicerPETTumorSegmentationLogic::Clone(OSFGraphType::Pointer graph)
{ //Copy the graph itself for MRML node undo/redo
  if (graph.IsNull())
    return OSFGraphType::Pointer(0);
  typedef itk::CloneOSFGraphFilter<OSFGraphType> CloneGraphFilterType;
  CloneGraphFilterType::Pointer cloner = CloneGraphFilterType::New();
  cloner->SetInput(graph);
  cloner->Update();
  return cloner->GetOutput();
}
  
