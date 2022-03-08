/*==============================================================================

 Program: PETTumorSegmentation

 (c) Copyright University of Iowa All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ==============================================================================*/

#ifndef __itkSealingSegmentationMergerImageFilter_h
#define __itkSealingSegmentationMergerImageFilter_h

#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkInterpolateImageFunction.h"
#include "itkConceptChecking.h"
#include "itkImageToImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include "itkSliceBySliceImageFilter.h"
#include "itkGrayscaleFillholeImageFilter.h"

namespace itk
{
/**\class SealingSegmentationMergerImageFilter
 * \brief Merges two segmentation volumes together, with one explicitly of a new object.
 * \date	12/9/2014
 * \author	Christian Bauer, Markus Van Tol
 * Merges a new object segmentation with the segmentation of other objects.  If set, avoids overwriting
 * old objects and seals gaps between the new object and old objects or more of the new object.
 * Template parameters for class SealingSegmentationMergerImageFilter:
 *
 * - TInputImage = The image type of the new segmentation to be incorporated.
 * - TUptakeImage = The image type of the existing segmentation to be added to.
 * - TOutputImage = The image type of the merged output.
 */
template <class TInputImage, class TUptakeImage, class TOutputImage>
class ITK_EXPORT SealingSegmentationMergerImageFilter : public ImageToImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class type aliases. */
  using Self = SealingSegmentationMergerImageFilter;
  using Superclass = ImageToImageFilter<TInputImage,TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  ITK_DISALLOW_COPY_AND_ASSIGN(SealingSegmentationMergerImageFilter);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SealingSegmentationMergerImageFilter, ImageToImageFilter);

  /** Some convenient type aliases. */
  using InputImageType = TInputImage;
  using InputImagePointer = typename InputImageType::ConstPointer;
  using InputImageRegionType = typename InputImageType::RegionType;
  using InputImagePixelType = typename InputImageType::PixelType;

  using UptakeImageType = TUptakeImage;
  using UptakeImagePointer = typename UptakeImageType::Pointer;
  using UptakeImageRegionType = typename UptakeImageType::RegionType;
  using UptakeImagePixelType = typename UptakeImageType::PixelType;

  using OutputImageType = TOutputImage;
  using OutputImagePointer = typename OutputImageType::Pointer;
  using OutputImageRegionType = typename OutputImageType::RegionType;
  using OutputImagePixelType = typename OutputImageType::PixelType;

  using PointType = typename TOutputImage::PointType ;

  /** ImageDimension enumeration */
  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;
  static constexpr unsigned int OutputImageDimension = TOutputImage::ImageDimension;
  
  itkSetObjectMacro(DataImage, UptakeImageType);
  itkGetObjectMacro(DataImage, UptakeImageType);

  itkSetObjectMacro(LabelImage, InputImageType);
  itkGetObjectMacro(LabelImage, InputImageType);

  itkSetMacro(Threshold, UptakeImagePixelType);
  itkGetMacro(Threshold, UptakeImagePixelType);

  itkSetMacro(Label, InputImagePixelType);
  itkGetMacro(Label, InputImagePixelType);

  itkSetMacro(PaintOver, bool);
  itkGetMacro(PaintOver, bool);

  itkSetMacro(Sealing, bool);
  itkGetMacro(Sealing, bool);

  itkSetMacro(NecroticRegion, bool);
  itkGetMacro(NecroticRegion, bool);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(ImageDimensionCheck,
      (Concept::SameDimensionOrMinusOne<itkGetStaticConstMacro(InputImageDimension),
                                        itkGetStaticConstMacro(OutputImageDimension)>));
  /** End concept checking */
#endif

protected:
  SealingSegmentationMergerImageFilter() = default;
  ~SealingSegmentationMergerImageFilter() override = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;

  void BeforeThreadedGenerateData() override;
  void DynamicThreadedGenerateData(const OutputImageRegionType& outputRegionForThread) override;

private:
  UptakeImagePixelType m_Threshold{ 0.0 };
  InputImagePixelType m_Label{ 1 } ;
  bool m_Sealing{ false };
  bool m_PaintOver{ false };
  bool m_NecroticRegion{ false };
  typename InputImageType::Pointer m_LabelImage;
  typename UptakeImageType::Pointer m_DataImage;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSealingSegmentationMergerImageFilter.txx"
#endif

#endif
