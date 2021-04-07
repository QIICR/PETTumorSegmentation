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

 ==============================================================================*/#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLPETTumorSegmentationParametersNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPETTumorSegmentationParametersNode);

//----------------------------------------------------------------------------
vtkMRMLPETTumorSegmentationParametersNode::vtkMRMLPETTumorSegmentationParametersNode()
{
  this->Label = 1;
  this->PaintOver = false;
  this->GlobalRefinementOn = true;
  this->LocalRefinementOn = false;
  this->PETVolumeReference = nullptr;
  this->CenterPointIndicatorListReference = nullptr;
  this->GlobalRefinementIndicatorListReference = nullptr;
  this->LocalRefinementIndicatorListReference = nullptr;
  this->SegmentationVolumeReference = nullptr;
  this->SegmentationReference = nullptr;
  this->SelectedSegmentID = nullptr;
  this->AssistCentering = true;
  this->Splitting = false;
  this->Sealing = false;
  this->DenoiseThreshold = false;
  this->LinearCost = false;
  this->NecroticRegion = false;
  this->OSFGraph = nullptr;
  this->InitialLabelMap = nullptr;
}

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
    //options
    this->SetLabel(node->GetLabel());
    this->SetPaintOver(node->GetPaintOver());
    this->SetGlobalRefinementOn(node->GetGlobalRefinementOn());
    this->SetLocalRefinementOn(node->GetLocalRefinementOn());
    this->SetPETVolumeReference(node->GetPETVolumeReference());
    this->SetCenterPointIndicatorListReference(node->GetCenterPointIndicatorListReference());
    this->SetGlobalRefinementIndicatorListReference(node->GetGlobalRefinementIndicatorListReference());
    this->SetLocalRefinementIndicatorListReference(node->GetLocalRefinementIndicatorListReference());
    this->SetSegmentationVolumeReference(node->GetSegmentationVolumeReference());
    this->SetSegmentationReference(node->GetSegmentationReference());
    this->SetSelectedSegmentID(node->GetSelectedSegmentID());
    this->SetAssistCentering(node->GetAssistCentering());
    this->SetSplitting(node->GetSplitting());
    this->SetSealing(node->GetSealing());
    this->SetDenoiseThreshold(node->GetDenoiseThreshold());
    this->SetLinearCost(node->GetLinearCost());
    this->SetNecroticRegion(node->GetNecroticRegion());

    //intermediate results
    this->SetCenterpoint(node->GetCenterpoint());
    this->SetOSFGraph(node->GetOSFGraph());
    this->SetInitialLabelMap(node->GetInitialLabelMap());
    this->SetHistogram(node->GetHistogram());
    this->SetHistogramRange(node->GetHistogramRange());
    this->SetHistogramMedian(node->GetHistogramMedian());
    this->SetCenterpointUptake(node->GetCenterpointUptake());
    this->SetThreshold(node->GetThreshold());
  }
  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
  //TODO: implement
}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " label=\"" << this->Label << "\"";

  if (this->PETVolumeReference != nullptr)
    {
    of << indent << " PETVolumeReference=\"" << this->PETVolumeReference << "\"";
    }
  if (this->CenterPointIndicatorListReference != nullptr)
    {
    of << indent << " centerPointIndicatorListReference=\"" << this->CenterPointIndicatorListReference << "\"";
    }
  if (this->GlobalRefinementIndicatorListReference != nullptr)
    {
    of << indent << " globalRefinementIndicatorListReference=\"" << this->GlobalRefinementIndicatorListReference << "\"";
    }
  if (this->LocalRefinementIndicatorListReference != nullptr)
    {
    of << indent << " localRefinementIndicatorListReference=\"" << this->LocalRefinementIndicatorListReference << "\"";
    }
  if (this->SegmentationVolumeReference != nullptr)
    {
    of << indent << " segmentationVolumeReference=\"" << this->SegmentationVolumeReference << "\"";
    }
  if (this->SegmentationReference != nullptr)
    {
    of << indent << " SegmentationReference=\"" << this->SegmentationReference << "\"";
    }
  if (this->SelectedSegmentID != nullptr)
    {
    of << indent << " SelectedSegmentID=\"" << this->SelectedSegmentID << "\"";
    }

  of << indent << " paintOver=\"" << this->PaintOver << "\"";
  of << indent << " globalRefinementOn=\"" << this->GlobalRefinementOn << "\"";
  of << indent << " localRefinementOn=\"" << this->LocalRefinementOn << "\"";
  of << indent << " assistCentering=\"" << this->AssistCentering << "\"";
  of << indent << " splitting=\"" << this->Splitting << "\"";
  of << indent << " sealing=\"" << this->Sealing << "\"";
  of << indent << " denoiseThreshold=\"" << this->DenoiseThreshold << "\"";
  of << indent << " linearCost=\"" << this->LinearCost << "\"";
  of << indent << " necroticRegion=\"" << this->NecroticRegion << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLPETTumorSegmentationParametersNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "label"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Label;
      }
    else if (!strcmp(attName, "PETVolumeReference"))
      {
      this->SetPETVolumeReference(attValue);
      }
    else if (!strcmp(attName, "CenterPointIndicatorListReference"))
      {
      this->SetCenterPointIndicatorListReference(attValue);
      }
    else if (!strcmp(attName, "globalRefinementIndicatorListReference"))
      {
      this->SetGlobalRefinementIndicatorListReference(attValue);
      }
    else if (!strcmp(attName, "paintOver"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->PaintOver;
      }
    else if (!strcmp(attName, "assistCentering"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AssistCentering;
      }
    else if (!strcmp(attName, "globalRefinementOn"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlobalRefinementOn;
      }
    else if (!strcmp(attName, "localRefinementOn"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LocalRefinementOn;
      }
    else if (!strcmp(attName, "splitting"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Splitting;
      }
    else if (!strcmp(attName, "sealing"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Sealing;
      }
    else if (!strcmp(attName, "denoiseThreshold"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DenoiseThreshold;
      }
    else if (!strcmp(attName, "linearCost"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LinearCost;
      }
    else if (!strcmp(attName, "necroticRegion"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->NecroticRegion;
      }
    }

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
  outfile << "CenterPointIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLFiducialListNode*>(GetScene()->GetNodeByID(CenterPointIndicatorListReference)))<<"\n"; // char *CenterPointIndicatorListReference;
  outfile << "GlobalRefinementIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLFiducialListNode*>(GetScene()->GetNodeByID(GlobalRefinementIndicatorListReference)))<<"\n"; // char *GlobalRefinementIndicatorListReference;
  outfile << "LocalRefinementIndicatorListReference="<<FiducialsInfo(static_cast<vtkMRMLFiducialListNode*>(GetScene()->GetNodeByID(LocalRefinementIndicatorListReference)))<<"\n"; // char *LocalRefinementIndicatorListReference;
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
std::string vtkMRMLPETTumorSegmentationParametersNode::FiducialsInfo(vtkMRMLFiducialListNode* fiducials)
{
  if (fiducials==nullptr) return std::string("");
  std::stringstream info;
  info << std::setprecision(20);
  info << fiducials->GetNumberOfFiducials();
  for (int i=0; i<fiducials->GetNumberOfFiducials(); ++i)
  {
    info << "," << fiducials->GetNthFiducialXYZ(i)[0];
    info << "," << fiducials->GetNthFiducialXYZ(i)[1];
    info << "," << fiducials->GetNthFiducialXYZ(i)[2];
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
