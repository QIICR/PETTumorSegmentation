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

// .NAME vtkSlicerPETTumorSegmentationLogic - logic for PET Tumor segmentation using Optimal Surface Finding (OSF) with Refinement
// .SECTION Description
// This class manages the processing logic to obtain and update the OSF segmentation
// The class is passive and does not listen to any node changes
// The user has to call the Apply... methods explicitly

#ifndef __vtkSlicerPETTumorSegmentationLogic_h
#define __vtkSlicerPETTumorSegmentationLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// ITK includes
#include <itkImage.h>
#include <itkMesh.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

// OSF includes
#include "itkOSFGraph.h"

// MRML includes

// STD includes
#include <cstdlib>
#include "vtkSlicerPETTumorSegmentationModuleLogicExport.h"
class vtkMRMLPETTumorSegmentationParametersNode;


/**\class vtkSlicerPETTumorSegmentationLogic
 * \brief Class with all logic methods for the PETTumorSegmentationEffect editor tool.
 * \date	12/9/2014
 * \author	Christian Bauer, Markus Van Tol
 * This class contains the logic methods for the PETTumorSegmentationEffect editor tool.  Most state
 * information is contained within the node itself, but the strong and weak watershed volumes, along
 * with some simple finger print variables to recognize when they are out fo date, are kept local in
 * order to reduce long term memory use and still avoid excessive recalculation. \n
 * External function calls should use Apply, ApplyGlobalRefinement, or ApplyLocalRefinement only in
 * order to make a complete segmentation. 
 */

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_PETTUMORSEGMENTATION_MODULE_LOGIC_EXPORT vtkSlicerPETTumorSegmentationLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPETTumorSegmentationLogic *New();
  vtkTypeMacro(vtkSlicerPETTumorSegmentationLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // user callable methods for OSF based segmentation
  // note: apply has been called before the refinement steps. Otherwise, there is nothing to refine.
  /** Called after making a center point.  Creates the default segmentation of the graph. */
  void Apply(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData);
  
  /** Called after making a global refineemnt point.  Changes the result around the segmentation. */
  void ApplyGlobalRefinement(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData);
  
  /** Called after making a local refinement point.  Changes the result in a narrow region. */
  void ApplyLocalRefinement(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData);
  
protected:
  vtkSlicerPETTumorSegmentationLogic();
  virtual ~vtkSlicerPETTumorSegmentationLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  
  
  
  // typedef internally utilized data representation
  typedef itk::Image<short, 3> LabelImageType;
  typedef itk::Image<float, 3> ScalarImageType;
  typedef unsigned long WatershedType;
  typedef itk::Image<WatershedType, 3> WatershedImageType;
  typedef ScalarImageType::IndexType IndexType;
  typedef ScalarImageType::PointType PointType;
  typedef ScalarImageType::RegionType RegionType;
  typedef itk::LinearInterpolateImageFunction<ScalarImageType> InterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<LabelImageType> LabelInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<WatershedImageType> WatershedInterpolatorType;
  typedef itk::OSFGraph<float> OSFGraphType;
  typedef OSFGraphType::OSFSurface OSFSurfaceType;
  typedef itk::Mesh<float, 3> MeshType;
  typedef std::vector<float> HistogramType;
  
  
  
  // methods for main processing steps
  /** Generates the graph and calculates the threshold. */
  bool InitializeOSFSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap); // intial setup for segmentation
  
  /** Sets the graph node costs based on the threshold. */
  void UpdateGraphCostsGlobally(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap); // incorporate global refinement information
  
  /** Modifies the graph node costs based on the most recent local refinement point. */
  void UpdateGraphCostsLocally(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, bool renewOldPoints=false); // incorporate local refinement information
  
  

  
  /** Completes final steps of segmentation, including solving and modifying the label map. */
  void FinalizeOSFSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap); // update OSF segmentation and output
  
  // helper methods utilized by main processing steps
  /** Convertes the base label map from VTK to ITK.  Gets spacing information from the parameter node. */
  LabelImageType::Pointer ConvertLabelImageToITK(vtkMRMLPETTumorSegmentationParametersNode* node, vtkImageData* labelImageData);  // convert the copy of the image data to a useable form
  
  /** Checks if the input for the parameter node is valid. */
  bool ValidInput(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Gets the center point from the fiducial list into the parameter node.  Returns false if the center point is out of bounds or nonexistant. */
  bool CalculateCenterPoint(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer initialLabelMap);
  
  /** Returns the subvolume of the PET image around the center. */
  ScalarImageType::Pointer ExtractPETSubVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  /** Returns the isotropic subvolume of the PET image around the center. */
  ScalarImageType::Pointer ExtractPETSubVolumeIsotropic(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  /** Generates the strong and weak watershed volumes. */
  void GenerateWatershedImages(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petSubVolume);
  
  /** Instantiates the graph and all columns. */
  void CreateGraph(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Generates the histogram of values around the center point. */
  void ObtainHistogram(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  /** Completes the maximum flow step of the solution algorithm. */
  void MaxFlow(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Returns the solution mesh for the segmentation. */
  MeshType::Pointer GetSegmentationMesh(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Returns the voxelization of the mesh minus any culling. */
  LabelImageType::Pointer GetSegmentation(vtkMRMLPETTumorSegmentationParametersNode* node, MeshType::Pointer mesh, LabelImageType::Pointer initialLabelMap);
  
  /** Updates the label map volume to include the newly made segmentation.*/
  void UpdateLabelMap(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, LabelImageType::Pointer segmentation, LabelImageType::Pointer initialLabelMap);
  
  /** Calculates the threshold based on the histogram of voxels around the center point. */
  void CalculateThresholdHistogramBased(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  /** Calculates the threshold in the parameter node based on the global refinement point's location. */
  void CalculateThresholdPointLocationBased(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  
  
  
  // methods for local refinement node selection
  /** Finds the closest vertex to the target point p. */
  int GetClosestVertex(vtkMRMLPETTumorSegmentationParametersNode* node, const PointType& p);
  
  /** Finds the closest column on a vertex to the target point p. */
  int GetClosestColumnOnVertex(vtkMRMLPETTumorSegmentationParametersNode* node, const PointType& p, int vertexId);
  
  /** Adds the cost change from the most recent local refinement step. */
  void AddLocalRefinementCosts(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume, const PointType& refinementPoint, std::vector<bool> depth0ModifiedOverall, std::vector<bool> depth0ModifiedSequence);
  
  /** Finds the array of uptakes within range that best matches the initial template.*/
  int GetBestTemplateMatch(std::vector<float> vecTemplate, int idxTemplate, int len, std::vector<float> vecB, int range, float& matchingScore);
  
  // finger print-based methods to reduce memory use in MRML node and reduce time remaking utility volumes
  /** Updates the local finger print variables to reflect the parameter node. */
  void UpdateFingerPrint(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Checks if the current local finger print variables match the parameter node, allowing the local watershed volumes to be used. */
  bool CheckFingerPrint(vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Returns the PET volume in ITK format from the parameter node. */
  ScalarImageType::Pointer GetPETVolume(vtkMRMLPETTumorSegmentationParametersNode* node);

  /** Returns the strong watershed volume.  Generates it, if needed, otherwise uses the local copy. */
  WatershedImageType::Pointer GetStrongWatershedVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  /** Returns the weak watershed volume.  Generates it, if needed, otherwise uses the local copy. */
  WatershedImageType::Pointer GetWeakWatershedVolume(vtkMRMLPETTumorSegmentationParametersNode* node, ScalarImageType::Pointer petVolume);
  
  // utility methods for multi-threading
  /** Determines the median uptake at a certain shell level and sets it in the parameter node. */
  static void GetMedianUptakeForShell(int shellId, vtkMRMLPETTumorSegmentationParametersNode* node, std::vector<float>& shellUptakes, InterpolatorType::Pointer interpolator);
  
  /** Sets the base cost and adds cost adjustments based on no refinement to the graph at the vertex.  Requires the parameter node and interpolators for the PET image, label volume, and watershed volumes. */
  static void SetGlobalGraphCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, InterpolatorType::Pointer interpolator, LabelInterpolatorType::Pointer labelInterpolator, WatershedInterpolatorType::Pointer strongWatershedInterpolator, WatershedInterpolatorType::Pointer weakWatershedInterpolator);
  
  /** Sets the costs on the graph at the vertex based on the threshold calculated.  Requires the parameter node and the uptake at the nodes. */
  static void SetGlobalBaseGraphCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues);
  
  /** Adds the costs at the vertex for label avoidance.  Requires the parameter node, the uptake at the nodes, and an interpolator for the label volume. */
  static void AddLabelAvoidanceCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues, LabelInterpolatorType::Pointer labelInterpolator);
  
  /** Adds the necrotic costs for no label avoidance to the vertex of choice.  Requires the parameter node and an interpolator for the label volume. */
  static void AddDefaultNecroticCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, LabelInterpolatorType::Pointer labelInterpolator);
  
  /** Adds the costs to the parameter node's graph for splitting mode to the vertex of choice.  Requires the parameter node, uptake values and interpolators for the strong and weak watershed volumes. */
  static void AddSplittingCostsForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, const std::vector<float>& uptakeValues, WatershedInterpolatorType::Pointer strongWatershedInterpolator, WatershedInterpolatorType::Pointer weakWatershedInterpolator);
  
  /** Samples the uptake at each of the nodes on the given column, given the parameter node and the interpolator of choice. */
  template <typename valueType, class ImageInterpolatorType>
  static std::vector<valueType> SampleColumnPoints(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node, typename ImageInterpolatorType::Pointer interpolator, valueType defaultValue=0);
  
  /** Builds the indexed column on the graph contained in the parameter node. */
  static void BuildColumnForVertex(int vertexId, vtkMRMLPETTumorSegmentationParametersNode* node);
  
  /** Makes a deep copy of the graph object. */
  static OSFGraphType::Pointer Clone(OSFGraphType::Pointer graph);
  
  
  
private:
  vtkSlicerPETTumorSegmentationLogic(const vtkSlicerPETTumorSegmentationLogic&); // purposely not implemented
  void operator=(const vtkSlicerPETTumorSegmentationLogic&); // purposely not implemented
  
  /** Converts a VTK volume into an ITK volume. */
  template <class ITKImageType>
  typename ITKImageType::Pointer convert2ITK(vtkSmartPointer<vtkImageData> vtkVolume);
  PointType convert2ITK(const float* coordinate);
  
  /** Converts an ITK volume into a VTK volume. */
  template <class ITKImageType>
  vtkSmartPointer<vtkImageData> convert2VTK(typename ITKImageType::Pointer itkVolume, const int TypeVTK);
  
  /** Debug method.  Used to write a volume to a file. */
  template <class ITKImageType>
  void writeImage(typename ITKImageType::Pointer itkVolume, char* filename);
  
  /** Debug method.  Used to write a mesh to a file. */
  template <class ITKMeshType>
  void writeMesh(typename ITKMeshType::Pointer itkMesh, char* filename);
  
  
  
  
  /** Determines the density of the spherical mesh.  At density of 4, there are 1026 vertices. */
  const static int meshResolution;
  
  /** Determines the radius of the spherical mesh.  60 mm is sufficiently large for the vast majority of cases. */
  const static float meshSphereRadius;
  
  /** Determines the distance between nodes in the column, IE the minimum difference in boundary placement.  1 mm is significantly lower than the lowest voxel dimension encountered. */
  const static float columnStepSize;
  
  /** Determines the maximum change in boundary between columns.  5 mm is sufficient to prevent major discontinuity, while allowing oddly-shaped objects with poor center placement. */
  const static int hardSmoothnessConstraint;
  
  /** Determines the penalty in cost for a difference in surface between columns in standard mode.  0.005 is low, sufficient mostly for a tiebreaker and minor smoothing for the more oddly shaped lesions that primary tumors tend to be. */
  const static float softSmoothnessPenalty;
  
  /** Determines the penalty in cost for a difference in surface between columns in splitting mode.  0.05 is much higher, useful for cutting off stray parts to help with the purpose of splitting, as it is typically used for lymph nodes. */
  const static float softSmoothnessPenaltySplitting;
  
  /** Determines the first node available as a surface, in order to avoid trivially small objects.  3, paried with 1 mm columnStepSize, is sufficient to be around a voxel in each direction, avoiding single-voxel solutions.*/
  const static int minNodeRejections;
  
  /** Maximum node choosable for local refinement.  56 is chosen due to the columnStepSize of 1.0, radius of 60, and templateMatchingHalfLength of 3, so that the comparison array for local refinement never stretches above node 59.*/
  const static int maxNodeRefinement;
  
  /** Determines the cost added to reject a node.  6, compared with the typical base cost scaling from 0.0 to 1.0, is sufficient to reject nodes that aren't specifically refined to.*/
  const static float rejectionValue;
  
  /** Determines the number of bins for the histogram processing.  100, with typical SUV values from 0 to 10 and at most to somewhere between 20 or 30, gives sufficient density to make a reasonable cost curve. */
  const static int numHistogramBins;
  
  /** Determines the distance to search for a better center point if assist centering is active.  7.0 mm is aroud 2-3 voxels, which is enough to significantly improve consistency on small-to-medium lesions, and somewhat help on large ones, all without much danger of moving to another lesion for small ones.*/
  const static float centeringRange;
  
  /** Double the value and add 1 to get the number of nodes in a comparison array for local refinement.  A value of 3 makes a total of 7 nodes in the array for comparison.  This is sufficient for comparing approximate threshold and general features near a refinement point. */
  const static int templateMatchingHalfLength;
  
  /** The portion of the total uptake of the original array a possible refinement array's difference with it must be below to be considered similar.  A higher value makes refinement spread more, a lower value makes it spread less.  0.05 is sufficient to have a strict requirement to reject unlike arrays, but still allow it to spread to similar constructs nearby. */
  const static float similarityThresholdFactor;
  
  
  
  /** The name of the most recent PET volume node.  Stored to recognize when watershed volumes need not be recalculated. */
  std::string volumeFingerPrint;
  
  /** The coordinates of the most recent center point.  Stored to recognize when watershed volumes need not be recalculated. */
  std::vector<float> centerFingerPrint;
  
  /** A pointer to the most recent strong watershed volume.  Saved to avoid lengthy recalculation when it is avoidable. */
  WatershedImageType::Pointer StrongWatershedVolume_saved;
  
  /** A pointer to the most recent weak watershed volume.  Saved to avoid lengthy recalculation when it is avoidable. */
  WatershedImageType::Pointer WeakWatershedVolume_saved;
  
};

#endif
