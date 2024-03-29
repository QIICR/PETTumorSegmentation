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

#ifndef __vtkMRMLPETTumorSegmentationParametersNode_h
#define __vtkMRMLPETTumorSegmentationParametersNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include "vtkStringArray.h"
//#include "vtkImageStash.h"

// ITK includes
#include <itkImage.h>
#include <itkMesh.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkLabelMap.h>
#include <itkLabelObject.h>

// OSF includes
#include "../Logic/itkOSFGraph.h"

// STL includes
#include <string>
#include <vector>

// for debugging
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLScene.h>
#include <vtkOrientedImageData.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkSlicerSegmentationsModuleLogic.h>
#include <vtkSegmentation.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
class vtkSlicerSegmentationsModuleLogic;
#include <qSlicerApplication.h>

#include "vtkSlicerPETTumorSegmentationModuleMRMLExport.h"

class VTK_SLICER_PETTUMORSEGMENTATION_MODULE_MRML_EXPORT vtkMRMLPETTumorSegmentationParametersNode : public vtkMRMLNode
{
  public:
  static vtkMRMLPETTumorSegmentationParametersNode *New();
  vtkTypeMacro(vtkMRMLPETTumorSegmentationParametersNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Clear stored processing results
  void Clear();

  // Description:
  // Create instance
  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  // Description:
  // Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "PETTumorSegmentationParametersNode"; };

  vtkGetMacro ( Label, short );
  vtkSetMacro ( Label, short );

  vtkGetMacro ( PaintOver, bool );
  vtkSetMacro ( PaintOver, bool );

  vtkGetMacro ( GlobalRefinementOn, bool );
  vtkSetMacro ( GlobalRefinementOn, bool );
  vtkGetMacro ( LocalRefinementOn, bool );
  vtkSetMacro ( LocalRefinementOn, bool );
  bool GetNoRefinementOn()  { return !(LocalRefinementOn || GlobalRefinementOn); }

  vtkGetStringMacro ( PETVolumeReference );
  vtkSetStringMacro ( PETVolumeReference );

  vtkGetStringMacro ( CenterPointIndicatorListReference );
  vtkSetStringMacro ( CenterPointIndicatorListReference );

  vtkGetStringMacro ( GlobalRefinementIndicatorListReference );
  vtkSetStringMacro ( GlobalRefinementIndicatorListReference );

  vtkGetStringMacro ( LocalRefinementIndicatorListReference );
  vtkSetStringMacro ( LocalRefinementIndicatorListReference );

  vtkGetStringMacro ( SegmentationVolumeReference );
  vtkSetStringMacro ( SegmentationVolumeReference );

  vtkGetStringMacro ( SegmentationReference );
  vtkSetStringMacro ( SegmentationReference );

  vtkGetStringMacro(SelectedSegmentID);
  vtkSetStringMacro(SelectedSegmentID);

  vtkGetMacro ( AssistCentering, bool );
  vtkSetMacro ( AssistCentering, bool );

  vtkGetMacro ( Splitting, bool );
  vtkSetMacro ( Splitting, bool );

  vtkGetMacro ( Sealing, bool );
  vtkSetMacro ( Sealing, bool );

  vtkGetMacro ( DenoiseThreshold, bool );
  vtkSetMacro ( DenoiseThreshold, bool );

  vtkGetMacro ( LinearCost, bool );
  vtkSetMacro ( LinearCost, bool );

  vtkGetMacro ( NecroticRegion, bool );
  vtkSetMacro ( NecroticRegion, bool );

  using LabelImageType = itk::Image<short, 3>;
  using LabelMapType = itk::LabelMap< itk::LabelObject< short, 3 > >;
  using ScalarImageType = itk::Image<float, 3>;
  using IndexType = ScalarImageType::IndexType;
  using PointType = ScalarImageType::PointType;
  using GraphType = itk::OSFGraph<float>;
  using MeshType = itk::Mesh<float, 3>;
  using HistogramType = std::vector<float>;
  using WatershedPixelType = unsigned long;
  using WatershedImageType = itk::Image<WatershedPixelType, 3>;

  void SetCenterpoint(PointType index) {Centerpoint = index;};
  PointType GetCenterpoint() {return Centerpoint;};
  float GetCenterpointX() { return Centerpoint[0];};
  float GetCenterpointY() { return Centerpoint[1];};
  float GetCenterpointZ() { return Centerpoint[2];};

  void SetHistogram(HistogramType hist) {Histogram = hist;};
  const HistogramType& GetHistogram() {return Histogram;};

  void SetHistogramRange(float range) {HistogramRange = range;};
  float GetHistogramRange() const {return HistogramRange;};

  void SetHistogramMedian(float value) {HistogramMedian = value;};
  float GetHistogramMedian() const {return HistogramMedian;};

  void SetCenterpointUptake(float value) {CenterpointUptake = value;};
  float GetCenterpointUptake() {return CenterpointUptake;};

  void SetThreshold(float threshold) {Threshold = threshold;};
  float GetThreshold() {return Threshold;};

  void SetOSFGraph(GraphType::Pointer graph) {OSFGraph = graph;};
  GraphType::Pointer GetOSFGraph() {return OSFGraph;};

  void SetInitialLabelMap(LabelImageType::Pointer labelMap) {InitialLabelMap = labelMap;};
  LabelImageType::Pointer GetInitialLabelMap() {return InitialLabelMap;};
  void ClearInitialLabelMap() {InitialLabelMap = nullptr;};

  // for debugging
  virtual void WriteTXT(const char* filename);

 protected:

  vtkMRMLPETTumorSegmentationParametersNode();
  ~vtkMRMLPETTumorSegmentationParametersNode() override;
  
  vtkMRMLPETTumorSegmentationParametersNode(const vtkMRMLPETTumorSegmentationParametersNode&);
  void operator=(const vtkMRMLPETTumorSegmentationParametersNode&);

  // segmentation parameters
  /** Current label being applied. */
  short Label{ 1 };

  /** Whether or not the new label should overwrite existing labels. */
  bool PaintOver{ false };

  /** Whether or not global refinement is currently set to be applied when refining. */
  bool GlobalRefinementOn{ true };

  /** Whether or not local refinement is currently set to be applied when refining. */
  bool LocalRefinementOn{ false };

  /** MRML node ID string for the PET volume node.*/
  char *PETVolumeReference{ nullptr };

  /** MRML node ID string for the center point fiducial list node.*/
  char *CenterPointIndicatorListReference{ nullptr };

  /** MRML node ID string for the global refinement point fiducial list node.*/
  char *GlobalRefinementIndicatorListReference{ nullptr };

  /** MRML node ID string for the local refinement point fiducial list node.*/
  char *LocalRefinementIndicatorListReference{ nullptr };

  /** MRML node ID string for the segmentation label volume node.*/
  char *SegmentationVolumeReference{ nullptr };

  /** MRML node ID string for the segmentation node.*/
  char *SegmentationReference{ nullptr };

  /** ID string for the segment.*/
  char* SelectedSegmentID{ nullptr };

  /** Whether or not the center point will be adjusted for the segmentation.*/
  bool AssistCentering{ true };

  /** Whether or not to apply modified splitting costs and penalties for the segmentation.*/
  bool Splitting{ false };

  /** Whether or not to seal the segmentation after voxelization.*/
  bool Sealing{ false };

  /** Whether or not to calculate the threshold for the segmentation based on a median-filtered image.*/
  bool DenoiseThreshold{ false };

  /** Whether or not to set the low-uptake end of the cost function linearly.*/
  bool LinearCost{ false };

  /** Whether or not to apply the segmentation in necrotic mode.*/
  bool NecroticRegion{ false };

  // intermediate processing results of each segmentation refinement step that will stored for undo-redo operations
  /** The center point after any recentering.*/
  PointType Centerpoint;

  /** The intial label map before starting a segmentation of the current lesion. */
  LabelImageType::Pointer InitialLabelMap{ nullptr };

  /** The graph structure with all costs and edges.*/
  GraphType::Pointer OSFGraph{ nullptr };

  /** The histogram of the region around the center.*/
  HistogramType Histogram;

  /** The range of values for the histogram.*/
  float HistogramRange;

  /** The median value of the histogram.*/
  float HistogramMedian;

  /** The uptake value at the center point, linearly interpolated.*/
  float CenterpointUptake;

  /** The threshold currently in use for cost setting.*/
  float Threshold;

private:
  // for debugging
  std::string VolumeInfo(vtkMRMLScalarVolumeNode* volume);
  template <class ITKImageType> std::string VolumeInfoITK(typename ITKImageType::Pointer image);
  std::string FiducialsInfo(vtkMRMLMarkupsFiducialNode* fiducials);
  typename LabelImageType::Pointer ConvertSegmentationToITK();
  template <class ITKImageType> typename ITKImageType::Pointer convert2ITK(vtkSmartPointer<vtkImageData> vtkVolume);

};

#endif
