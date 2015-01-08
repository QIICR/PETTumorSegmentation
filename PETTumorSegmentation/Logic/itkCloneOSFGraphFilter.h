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

#ifndef _itkCloneOSFGraphFilter_h
#define _itkCloneOSFGraphFilter_h

#include "itkOSFGraphToOSFGraphFilter.h"

namespace itk
{

/** \class CloneOSFGraphFilter
 * \brief Create a deep copy of an OSFGraph.
 * \date	12/9/2014
 * \author	Christian Bauer
 * Create a deep copy of an OSFGraph. CONTINUE
 * Template parameters for class CloneOSFGraphFilter:
 *
 * - TOSFGraph = The graph type of the input to clone.
 */
template <class TOSFGraph>
class ITK_EXPORT CloneOSFGraphFilter : public OSFGraphToOSFGraphFilter<TOSFGraph,TOSFGraph>
{
public:
  typedef CloneOSFGraphFilter Self;
  typedef OSFGraphToOSFGraphFilter<TOSFGraph,TOSFGraph> Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro( Self );
  itkTypeMacro( CloneOSFGraphFilter, OSFGraphToOSFGraphFilter );
  
protected:
  /** Constructor for use by New() method. */
  CloneOSFGraphFilter();
  ~CloneOSFGraphFilter() {};
  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  
  virtual void GenerateData();
  
private:
  CloneOSFGraphFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
}; // end class CloneOSFGraphFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCloneOSFGraphFilter.txx"
#endif

#endif

