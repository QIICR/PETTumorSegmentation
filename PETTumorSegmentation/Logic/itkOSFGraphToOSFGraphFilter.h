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

#ifndef _itkOSFGraphToOSFGraphFilter_h
#define _itkOSFGraphToOSFGraphFilter_h

#include "itkOSFGraphSource.h"

namespace itk
{

/**\class SealingSegmentationMergerImageFilter
 * \brief
 * \date	12/9/2014
 * \author	Christian Bauer
 * The superclass for any filter that takes one OSF graph as input and gives another as output.
 * By default, copies all input graph data to the output graph.
 * Template parameters for class SimpleOSFGraphBuilderFilter:
 *
 * - TInputOSFGraph = The graph type of the input.
 * - TOutputOSFGraph = The graph type of the output.
 */
 
template <class TInputOSFGraph, class TOutputOSFGraph>
class ITK_EXPORT OSFGraphToOSFGraphFilter : public OSFGraphSource<TOutputOSFGraph>
{
public:
  typedef OSFGraphToOSFGraphFilter Self;
  typedef OSFGraphSource<TOutputOSFGraph> Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro( Self );
  itkTypeMacro( OSFGraphToOSFGraphFilter, OSFGraphSource );
  
  typedef TInputOSFGraph InputOSFGraphType;
  typedef typename InputOSFGraphType::ConstPointer InputOSFGraphConstPointer;
  
  typedef TOutputOSFGraph OutputOSFGraphType;
  typedef typename OutputOSFGraphType::Pointer OutputOSFGraphPointer;
    
  virtual void SetInput(unsigned int idx, const InputOSFGraphType* input); // Set the input image of this process object.
  virtual const InputOSFGraphType* GetInput(unsigned int idx) const; // Get the input image of this process object.
  virtual void SetInput( const InputOSFGraphType* input) {this->SetInput(0, input);}; // Set the input image of this process object.
  virtual const InputOSFGraphType* GetInput() const {return this->GetInput(0);}; // Get the input image of this process object.
  
  OutputOSFGraphType* GetOutput(unsigned int idx); // Get the mesh output of this process object.
  OutputOSFGraphType* GetOutput(void) {return this->GetOutput(0);}; // Get the mesh output of this process object.

protected:
  /** Constructor for use by New() method. */
  OSFGraphToOSFGraphFilter();
  ~OSFGraphToOSFGraphFilter() {};
  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  
  typedef typename OutputOSFGraphType::OSFSurface OSFSurface;
  typedef typename OSFSurface::Pointer OSFSurfacePointer;
  typedef typename OSFSurface::ConstPointer OSFSurfaceConstPointer;
  virtual void CopyInputOSFGraphToOutputOSFGraphSurfaces();
  virtual void CopyInputOSFGraphToOutputOSFGraphSurface(OSFSurfaceConstPointer inputOSFSurface, OSFSurfacePointer outputOSFSurface);
  virtual void CopyInputOSFGraphToOutputOSFGraphGraph();
  // todo: add more copy functions as required
  
private:
  OSFGraphToOSFGraphFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
}; // end class OSFGraphToOSFGraphFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFGraphToOSFGraphFilter.txx"
#endif

#endif

