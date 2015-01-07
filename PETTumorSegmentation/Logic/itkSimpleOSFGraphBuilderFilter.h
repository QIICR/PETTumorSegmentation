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

#ifndef _itkSimpleOSFGraphBuilderFilter_h
#define _itkSimpleOSFGraphBuilderFilter_h

#include "itkOSFGraphToOSFGraphFilter.h"

namespace itk
{
/**\class SimpleOSFGraphBuilderFilter
 * \brief Generates the solvable graph information for an OSF graph object.
 * \date	12/9/2014
 * \author	Christian Bauer
 * Given an OSF graph with the costs and locations of the nodes and columns, this builds an accompanying
 * maximum-flow-solvable graph type, with the source and sink nodes in order to generate the solution to
 * the surface. \n
 * This also applies the hard and soft smoothness requirements.
 * Template parameters for class SimpleOSFGraphBuilderFilter:
 *
 * - TInputOSFGraph = The graph type of the input.
 * - TOutputOSFGraph = The graph type of the output.
 */
template <class TInputOSFGraph, class TOutputOSFGraph>
class ITK_EXPORT SimpleOSFGraphBuilderFilter : public OSFGraphToOSFGraphFilter<TInputOSFGraph,TOutputOSFGraph>
{
public:
  typedef SimpleOSFGraphBuilderFilter Self;
  typedef OSFGraphToOSFGraphFilter<TInputOSFGraph,TOutputOSFGraph> Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro( Self );
  itkTypeMacro( SimpleOSFGraphBuilderFilter, OSFGraphToOSFGraphFilter );
  
  typedef TInputOSFGraph InputOSFGraphType;
  typedef typename InputOSFGraphType::ConstPointer InputOSFGraphConstPointer;
  
  typedef TOutputOSFGraph OutputOSFGraphType;
  typedef typename OutputOSFGraphType::Pointer OutputOSFGraphPointer;
  
  itkSetMacro( SmoothnessConstraint, unsigned int );
  itkGetMacro( SmoothnessConstraint, unsigned int );
  
  itkSetMacro( SoftSmoothnessPenalty, double );
  itkGetMacro( SoftSmoothnessPenalty, double ); 
    
protected:
  /** Constructor for use by New() method. */
  SimpleOSFGraphBuilderFilter();
  ~SimpleOSFGraphBuilderFilter() {};
  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  
  virtual void GenerateData();
  
  typedef typename OutputOSFGraphType::VertexIdentifier SurfaceIdentifier;
  typedef typename OutputOSFGraphType::OSFSurface OSFSurface;
  typedef typename OSFSurface::VertexIdentifier VertexIdentifier;
  virtual void CreateNodesForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId);
  virtual void CreateIntraColumnArcsForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId);
  virtual void CreateInterColumnArcsForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId);
  
  // note: shanhui said that some people say the value has to be a large negative number
  // but he did not experience any negative effects
  // this behavior may probably depend on the range of the actual cost values
  const typename OutputOSFGraphType::GraphCosts m_Infinity;
  const typename OutputOSFGraphType::GraphCosts m_ColumnBasedNodeWeight;
  
  unsigned int m_SmoothnessConstraint;
  double m_SoftSmoothnessPenalty;
  
private:
  SimpleOSFGraphBuilderFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
}; // end class SimpleOSFGraphBuilderFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSimpleOSFGraphBuilderFilter.txx"
#endif

#endif

