/*==============================================================================

Program: PETTumorSegmentation

Portions (c) Copyright University of Iowa All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

==============================================================================*/

#ifndef __itkSealingSegmentationMergerImageFilter_txx
#define __itkSealingSegmentationMergerImageFilter_txx

#include "itkSealingSegmentationMergerImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include <itkConstNeighborhoodIterator.h>
#include <itkImageDuplicator.h>
#include <cmath>
#include <string>
#include <iostream>

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

namespace itk
{

/**
 * Constructor
 */

template <class TInputImage, class TUptakeImage, class TOutputImage>
SealingSegmentationMergerImageFilter<TInputImage, TUptakeImage, TOutputImage>
::SealingSegmentationMergerImageFilter() :
  m_Threshold(0.0),
  m_Label(1),
  m_Sealing(false),
  m_PaintOver(false)
{
}

template <class TInputImage, class TUptakeImage, class TOutputImage>
void
SealingSegmentationMergerImageFilter<TInputImage, TUptakeImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  // create output image
  InputImagePointer input = this->GetInput();
  OutputImagePointer output = this->GetOutput();
  output->CopyInformation(input);
  output->Allocate();
}

template <class TInputImage, class TUptakeImage, class TOutputImage>
void
SealingSegmentationMergerImageFilter<TInputImage, TUptakeImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId)
{
  (void) threadId; // mute unused parameter warning; we use outputRegionForThread instead of threadId
  typedef itk::ConstNeighborhoodIterator<InputImageType> InputNeighborhoodIteratorType;
  typedef itk::ImageRegionConstIterator<InputImageType> InputIteratorType;
  typedef itk::ImageRegionIterator<UptakeImageType> UptakeIteratorType;
  typedef itk::ImageRegionIterator<OutputImageType> OutputIteratorType;
  
  typedef typename InputNeighborhoodIteratorType::RadiusType InputRadiusType;
  InputRadiusType radius;
  radius.Fill(1);
  
  InputNeighborhoodIteratorType inputIt(radius, this->GetInput(), outputRegionForThread); //iterator for newly created label
  InputNeighborhoodIteratorType segmentationIt(radius, this->GetLabelImage(), outputRegionForThread); //iterator for existing labels
  UptakeIteratorType uptakeIt(this->GetDataImage(), outputRegionForThread);
  OutputIteratorType outputIt(this->GetOutput(), outputRegionForThread);
  while (!outputIt.IsAtEnd()) //Zero out any data left over from old memory
  {
    outputIt.Set(0);
    ++outputIt;
  }
  outputIt.GoToBegin();
  while (!inputIt.IsAtEnd())  //Apply the existing labels and the actual sealing
  {
    if (inputIt.GetCenterPixel()>0 && (segmentationIt.GetCenterPixel()==0 || m_PaintOver))  //If the new label is to be applied and either the existing one is blank or set to be painted over, then apply the new label
      outputIt.Set(m_Label);
    else if (segmentationIt.GetCenterPixel()>0) //Otherwise, if there's an existing label, apply it
      outputIt.Set(segmentationIt.GetCenterPixel());
    else if (m_Sealing && (uptakeIt.Get()>=m_Threshold || m_NecroticRegion))  //Otherwise, if sealing's active and the voxel is either above the threshold or the region is marked necrotic, thereby ignoring the threshold, then check if the voxel must be sealed
    {
      for (int dim=0; dim<3; ++dim) //check each dimension
      {
        if (inputIt.GetPrevious(dim)>0 && (segmentationIt.GetNext(dim)>0 || inputIt.GetNext(dim)>0))  //If previous in this dimension is a new label and next is a new or old label, seal with new label
          outputIt.Set(m_Label);
        else if (inputIt.GetNext(dim)>0 && (segmentationIt.GetPrevious(dim)>0 || inputIt.GetPrevious(dim)>0)) //If next in this dimension is a new label and previous is a new or old label, seal with new label
          outputIt.Set(m_Label);
      }       
    }
    else
      outputIt.Set(0);
      
    ++inputIt;
    ++segmentationIt;
    ++uptakeIt;
    ++outputIt;    
  }  
}

template <class TInputImage, class TUptakeImage, class TOutputImage>
void SealingSegmentationMergerImageFilter<TInputImage, TUptakeImage, TOutputImage>::
PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

} // end namespace itk


#endif
