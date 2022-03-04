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

#ifndef _itkCenterNormalColumnBuilderFilter_h
#define _itkCenterNormalColumnBuilderFilter_h

#include "itkOSFGraphToOSFGraphFilter.h"

namespace itk
{
/**\class CenterNormalColumnBuilderFilter
 * \brief Builds the columns in the graph based on the vertex points of the existing graph.
 * \date	12/9/2014
 * \author	Christian Bauer, Markus Van Tol
 * Builds the columns in the graph based on the vertex points of the existing graph. CONTINUE
 * Template parameters for class CenterNormalColumnBuilderFilter:
 *
 * - TInputOSFGraph = The graph type of the input to build columns for.
 * - TOutputOSFGraph = The graph type of the input with built columns.
 */
template <class TInputOSFGraph, class TOutputOSFGraph>
class ITK_EXPORT CenterNormalColumnBuilderFilter : public OSFGraphToOSFGraphFilter<TInputOSFGraph,TOutputOSFGraph>
{
public:
  typedef CenterNormalColumnBuilderFilter Self;
  typedef OSFGraphToOSFGraphFilter<TInputOSFGraph,TOutputOSFGraph> Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro( Self );
  itkTypeMacro( CenterNormalColumnBuilderFilter, OSFGraphToOSFGraphFilter );
  
  typedef TInputOSFGraph InputOSFGraphType;
  typedef typename InputOSFGraphType::ConstPointer InputOSFGraphConstPointer;
  
  typedef TOutputOSFGraph OutputOSFGraphType;
  typedef typename OutputOSFGraphType::Pointer OutputOSFGraphPointer;
  
  itkSetMacro( StepLength, float );
  itkGetMacro( StepLength, float );
  
  itkSetMacro( NumberOfSteps, unsigned int );
  itkGetMacro( NumberOfSteps, unsigned int );

  itkSetVectorMacro( CenterPoint, float, 3 );
  itkGetVectorMacro( CenterPoint, const float, 3 );

protected:
  /** Constructor for use by New() method. */
  CenterNormalColumnBuilderFilter();
  ~CenterNormalColumnBuilderFilter() override = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;
  
  void GenerateData() override;
  
  float m_StepLength;
  unsigned int m_NumberOfSteps;
  
  typedef typename OutputOSFGraphType::OSFSurface OSFSurface;
  typedef typename OSFSurface::VertexIdentifier VertexIdentifier;
  typedef typename OSFSurface::CellIdentifier CellIdentifier;
  virtual void BuildColumn(VertexIdentifier vertexId);
  
  //typedef Vector< float, ::itk::GetImageDimension<TImage>::ImageDimension > CenterPoint;
  float m_CenterPoint[3]; // todo: assuming fixed number of dimensions here
  //typedef Vector< float, ::itk::GetImageDimension<TImage>::ImageDimension > DirectionVector;
  typedef Vector< float, 3 > DirectionVector; // todo: assuming fixed number of dimensions here
  virtual DirectionVector GetNormal(const VertexIdentifier vertexId) const;
 
  std::vector< std::set<CellIdentifier> > m_VertexToCellLookupTable;
  virtual void BuildVertexToCellLookupTable();
  
private:
  CenterNormalColumnBuilderFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
}; // end class CenterNormalColumnBuilderFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCenterNormalColumnBuilderFilter.txx"
#endif

#endif

