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
 #include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLPETTumorSegmentationParametersNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPETTumorSegmentationParametersNode);

//----------------------------------------------------------------------------
vtkMRMLPETTumorSegmentationParametersNode::vtkMRMLPETTumorSegmentationParametersNode() = default;

//----------------------------------------------------------------------------
vtkMRMLPETTumorSegmentationParametersNode::~vtkMRMLPETTumorSegmentationParametersNode()
{
  this->SetPETVolumeReference ( nullptr );
  this->SetCenterPointIndicatorListReference ( nullptr );
  this->SetGlobalRefinementIndicatorListReference ( nullptr );
  this->SetLocalRefinementIndicatorListReference ( nullptr );
  this->SetSegmentationVolumeReference ( nullptr );
  this->SetSegmentationReference ( nullptr );
  this->SetSelectedSegmentID ( nullptr );
}

void vtkMRMLPETTumorSegmentationParametersNode::Clear()
{
  this->OSFGraph = nullptr;
  this->InitialLabelMap = nullptr;
  Histogram.clear();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLPETTumorSegmentationParametersNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLPETTumorSegmentationParametersNode *node = vtkMRMLPETTumorSegmentationParametersNode::SafeDownCast(anode);
  if (node)
  {
    vtkMRMLCopyBeginMacro(anode);

    //options
    vtkMRMLCopyIntMacro(Label);
    vtkMRMLCopyStringMacro(PETVolumeReference);
    vtkMRMLCopyStringMacro(CenterPointIndicatorListReference);
    vtkMRMLCopyStringMacro(GlobalRefinementIndicatorListReference);
    vtkMRMLCopyStringMacro(LocalRefinementIndicatorListReference);
    vtkMRMLCopyStringMacro(SegmentationVolumeReference);
    vtkMRMLCopyStringMacro(SegmentationReference);
    vtkMRMLCopyStringMacro(SelectedSegmentID);
    vtkMRMLCopyBooleanMacro(PaintOver);
    vtkMRMLCopyBooleanMacro(GlobalRefinementOn);
    vtkMRMLCopyBooleanMacro(LocalRefinementOn);
    vtkMRMLCopyBooleanMacro(AssistCentering);
    vtkMRMLCopyBooleanMacro(Splitting);
    vtkMRMLCopyBooleanMacro(Sealing);
    vtkMRMLCopyBooleanMacro(DenoiseThreshold);
    vtkMRMLCopyBooleanMacro(LinearCost);
    vtkMRMLCopyBooleanMacro(NecroticRegion);

    //intermediate results
    this->SetCenterpoint(node->GetCenterpoint());
    this->SetOSFGraph(node->GetOSFGraph());
    this->SetInitialLabelMap(node->GetInitialLabelMap());
    this->SetHistogram(node->GetHistogram());
    vtkMRMLCopyFloatMacro(HistogramRange);
    vtkMRMLCopyFloatMacro(HistogramMedian);
    vtkMRMLCopyFloatMacro(CenterpointUptake);
    vtkMRMLCopyFloatMacro(Threshold);

    vtkMRMLCopyEndMacro();
  }
  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintIntMacro(Label);
  vtkMRMLPrintStringMacro(PETVolumeReference);
  vtkMRMLPrintStringMacro(CenterPointIndicatorListReference);
  vtkMRMLPrintStringMacro(GlobalRefinementIndicatorListReference);
  vtkMRMLPrintStringMacro(LocalRefinementIndicatorListReference);
  vtkMRMLPrintStringMacro(SegmentationVolumeReference);
  vtkMRMLPrintStringMacro(SegmentationReference);
  vtkMRMLPrintStringMacro(SelectedSegmentID);
  vtkMRMLPrintBooleanMacro(PaintOver);
  vtkMRMLPrintBooleanMacro(GlobalRefinementOn);
  vtkMRMLPrintBooleanMacro(LocalRefinementOn);
  vtkMRMLPrintBooleanMacro(AssistCentering);
  vtkMRMLPrintBooleanMacro(Splitting);
  vtkMRMLPrintBooleanMacro(Sealing);
  vtkMRMLPrintBooleanMacro(DenoiseThreshold);
  vtkMRMLPrintBooleanMacro(LinearCost);
  vtkMRMLPrintBooleanMacro(NecroticRegion);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(label, Label);
  vtkMRMLWriteXMLStringMacro(PETVolumeReference, PETVolumeReference);
  vtkMRMLWriteXMLStringMacro(centerPointIndicatorListReference, CenterPointIndicatorListReference);
  vtkMRMLWriteXMLStringMacro(globalRefinementIndicatorListReference, GlobalRefinementIndicatorListReference);
  vtkMRMLWriteXMLStringMacro(localRefinementIndicatorListReference, LocalRefinementIndicatorListReference);
  vtkMRMLWriteXMLStringMacro(segmentationVolumeReference, SegmentationVolumeReference);
  vtkMRMLWriteXMLStringMacro(SegmentationReference, SegmentationReference);
  vtkMRMLWriteXMLStringMacro(SelectedSegmentID, SelectedSegmentID);
  vtkMRMLWriteXMLBooleanMacro(paintOver, PaintOver);
  vtkMRMLWriteXMLBooleanMacro(globalRefinementOn, GlobalRefinementOn);
  vtkMRMLWriteXMLBooleanMacro(localRefinementOn, LocalRefinementOn);
  vtkMRMLWriteXMLBooleanMacro(assistCentering, AssistCentering);
  vtkMRMLWriteXMLBooleanMacro(splitting, Splitting);
  vtkMRMLWriteXMLBooleanMacro(sealing, Sealing);
  vtkMRMLWriteXMLBooleanMacro(denoiseThreshold, DenoiseThreshold);
  vtkMRMLWriteXMLBooleanMacro(linearCost, LinearCost);
  vtkMRMLWriteXMLBooleanMacro(necroticRegion, NecroticRegion);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(label, Label);
  vtkMRMLReadXMLStringMacro(PETVolumeReference, PETVolumeReference);
  vtkMRMLReadXMLStringMacro(centerPointIndicatorListReference, CenterPointIndicatorListReference);
  vtkMRMLReadXMLStringMacro(globalRefinementIndicatorListReference, GlobalRefinementIndicatorListReference);
  vtkMRMLReadXMLStringMacro(localRefinementIndicatorListReference, LocalRefinementIndicatorListReference);
  vtkMRMLReadXMLStringMacro(segmentationVolumeReference, SegmentationVolumeReference);
  vtkMRMLReadXMLStringMacro(SegmentationReference, SegmentationReference);
  vtkMRMLReadXMLStringMacro(SelectedSegmentID, SelectedSegmentID);
  vtkMRMLReadXMLBooleanMacro(paintOver, PaintOver);
  vtkMRMLReadXMLBooleanMacro(globalRefinementOn, GlobalRefinementOn);
  vtkMRMLReadXMLBooleanMacro(localRefinementOn, LocalRefinementOn);
  vtkMRMLReadXMLBooleanMacro(assistCentering, AssistCentering);
  vtkMRMLReadXMLBooleanMacro(splitting, Splitting);
  vtkMRMLReadXMLBooleanMacro(sealing, Sealing);
  vtkMRMLReadXMLBooleanMacro(denoiseThreshold, DenoiseThreshold);
  vtkMRMLReadXMLBooleanMacro(linearCost, LinearCost);
  vtkMRMLReadXMLBooleanMacro(necroticRegion, NecroticRegion);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::WriteTXT(const char* filename)
{
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();
  std::ofstream outfile;
  outfile.open (filename);
  outfile << "Writing this to a file.\n";
  outfile << "Label="<<Label<<"\n"; // short Label;
  outfile << "PaintOver="<<PaintOver<<"\n"; // bool PaintOver;
  outfile << "GlobalRefinementOn="<<GlobalRefinementOn<<"\n"; // bool GlobalRefinementOn;
  outfile << "LocalRefinementOn="<<LocalRefinementOn<<"\n"; // bool LocalRefinementOn;
  outfile << "PETVolumeReference="<<VolumeInfo(static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID(PETVolumeReference)))<<"\n"; // char *PETVolumeReference;
  outfile << "CenterPointIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLMarkupsFiducialNode*>(GetScene()->GetNodeByID(CenterPointIndicatorListReference)))<<"\n"; // char *CenterPointIndicatorListReference;
  outfile << "GlobalRefinementIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLMarkupsFiducialNode*>(GetScene()->GetNodeByID(GlobalRefinementIndicatorListReference)))<<"\n"; // char *GlobalRefinementIndicatorListReference;
  outfile << "LocalRefinementIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLMarkupsFiducialNode*>(GetScene()->GetNodeByID(LocalRefinementIndicatorListReference)))<<"\n"; // char *LocalRefinementIndicatorListReference;
  outfile << "SegmentationVolumeReference="<<VolumeInfo(static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID(SegmentationVolumeReference)))<<"\n"; // char *SegmentationVolumeReference;
  outfile << "SegmentationReference="<<VolumeInfoITK<LabelImageType>(ConvertSegmentationToITK())<<"\n"; // char *SegmentationReference;
  outfile << "SelectedSegmentID="<<SelectedSegmentID<<"\n"; // char* SelectedSegmentID;
  outfile << "AssistCentering="<<AssistCentering<<"\n"; // bool AssistCentering;
  outfile << "Splitting="<<Splitting<<"\n"; // bool Splitting;
  outfile << "Sealing="<<Sealing<<"\n"; // bool Sealing;
  outfile << "DenoiseThreshold="<<DenoiseThreshold<<"\n"; // bool DenoiseThreshold;
  outfile << "LinearCost="<<LinearCost<<"\n"; // bool LinearCost;
  outfile << "NecroticRegion="<<NecroticRegion<<"\n"; // bool NecroticRegion;
  outfile << "Centerpoint="<<Centerpoint[0]<<","<<Centerpoint[1]<<","<<Centerpoint[2]<<","<<"\n"; // PointType Centerpoint;
  outfile << "InitialLabelMap="<<VolumeInfoITK<LabelImageType>(InitialLabelMap)<<"\n"; // LabelImageType::Pointer InitialLabelMap;
  //what to do with this? GraphType::Pointer OSFGraph;
  outfile << "Histogram="; for (unsigned int i=0; i<Histogram.size(); ++i) outfile<<","<<Histogram[i]; outfile<<"\n"; // HistogramType Histogram;
  outfile << "HistogramRange="<<Label<<"\n"; // float HistogramRange;
  outfile << "HistogramMedian="<<Label<<"\n"; // float HistogramMedian;
  outfile << "CenterpointUptake="<<Label<<"\n"; // float CenterpointUptake;
  outfile << "Threshold="<<Label<<"\n"; // float Threshold;
  outfile.close();
}

//----------------------------------------------------------------------------
std::string vtkMRMLPETTumorSegmentationParametersNode::VolumeInfo(vtkMRMLScalarVolumeNode* volume)
{
  if (volume==nullptr) return std::string("");
  std::stringstream info;
  info << std::setprecision(20);
  vtkSmartPointer<vtkImageData> image = volume->GetImageData();
  info << volume->GetOrigin()[0] << "," << volume->GetOrigin()[1] << "," << volume->GetOrigin()[2];
  info << "," << volume->GetSpacing()[0] << "," << volume->GetSpacing()[1] << "," << volume->GetSpacing()[2];
  info << "," << image->GetDimensions()[0] << "," << image->GetDimensions()[1] << "," << image->GetDimensions()[2];
  uint32_t sum = 0;
  void* bufferStart = image->GetScalarPointer();
  unsigned long numVoxels = image->GetDimensions()[0]*image->GetDimensions()[1]*image->GetDimensions()[2];
  void* bufferEnd = (char*)bufferStart+image->GetScalarSize()*(numVoxels-1);
  uint32_t* cur = (uint32_t*)(bufferStart);
  while(cur++<bufferEnd) sum += *cur;
  info << "," << sum;
  return info.str();
}

//----------------------------------------------------------------------------
template <class ITKImageType>
std::string vtkMRMLPETTumorSegmentationParametersNode::VolumeInfoITK(typename ITKImageType::Pointer image)
{
  if (image.IsNull()) return std::string("");
  std::stringstream info;
  info << std::setprecision(20);
  info << image->GetOrigin()[0] << "," << image->GetOrigin()[1] << "," << image->GetOrigin()[2];
  info << "," << image->GetSpacing()[0] << "," << image->GetSpacing()[1] << "," << image->GetSpacing()[2];
  info << "," << image->GetLargestPossibleRegion().GetSize()[0] << "," << image->GetLargestPossibleRegion().GetSize()[1] << "," << image->GetLargestPossibleRegion().GetSize()[2];
  uint32_t sum = 0;
  void* bufferStart = image->GetBufferPointer();
  unsigned long numVoxels = image->GetLargestPossibleRegion().GetSize()[0]*image->GetLargestPossibleRegion().GetSize()[1]*image->GetLargestPossibleRegion().GetSize()[2];
  void* bufferEnd = (char*)bufferStart+sizeof(typename ITKImageType::PixelType)*(numVoxels-1);
  uint32_t* cur = (uint32_t*)(bufferStart);
  while(cur++<bufferEnd) sum += *cur;
  info << "," << sum;
  return info.str();
}

//----------------------------------------------------------------------------
std::string vtkMRMLPETTumorSegmentationParametersNode::FiducialsInfo(vtkMRMLMarkupsFiducialNode* fiducials)
{
  if (fiducials==nullptr) return std::string("");
  std::stringstream info;
  info << std::setprecision(20);
  info << fiducials->GetNumberOfControlPoints();
  for (int i=0; i<fiducials->GetNumberOfControlPoints(); ++i)
  {
    info << "," << fiducials->GetNthControlPointPositionVector(i)[0];
    info << "," << fiducials->GetNthControlPointPositionVector(i)[1];
    info << "," << fiducials->GetNthControlPointPositionVector(i)[2];
  }
  return info.str();
}

//----------------------------------------------------------------------------
typename vtkMRMLPETTumorSegmentationParametersNode::LabelImageType::Pointer vtkMRMLPETTumorSegmentationParametersNode::ConvertSegmentationToITK()
{
  vtkMRMLScene* slicerMrmlScene = qSlicerApplication::application()->mrmlScene();//this->GetScene();//
  vtkMRMLSegmentationNode* vtkSegmentation = static_cast<vtkMRMLSegmentationNode*>(slicerMrmlScene->GetNodeByID( GetSegmentationReference() ));
  vtkMRMLScalarVolumeNode* vtkPetVolume = static_cast<vtkMRMLScalarVolumeNode*>(slicerMrmlScene->GetNodeByID( GetPETVolumeReference() ));
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry =
    vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(vtkPetVolume);

  vtkSmartPointer<vtkOrientedImageData> vtkLabelVolume = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkSegmentation->GenerateMergedLabelmapForAllSegments(vtkLabelVolume, vtkSegmentation::EXTENT_UNION_OF_SEGMENTS_PADDED, referenceGeometry);

  typedef itk::Image<short, 3> LabelImageType;
  LabelImageType::Pointer labelVolume = convert2ITK<LabelImageType>( vtkLabelVolume );
  labelVolume->SetSpacing( vtkLabelVolume->GetSpacing() );
  double origin2[3] = {-referenceGeometry->GetOrigin()[0], -referenceGeometry->GetOrigin()[1], referenceGeometry->GetOrigin()[2]};
  labelVolume->SetOrigin( origin2 );

  referenceGeometry->Delete();

  return labelVolume;
}

//---------------------------------------------------------------------------
template <class ITKImageType>
typename ITKImageType::Pointer
vtkMRMLPETTumorSegmentationParametersNode::convert2ITK(vtkSmartPointer<vtkImageData> vtkVolume)
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
