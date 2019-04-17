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

