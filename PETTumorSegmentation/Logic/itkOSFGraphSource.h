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

#ifndef _itkOSFGraphSource_h
#define _itkOSFGraphSource_h

#include <itkProcessObject.h>
#include "itkOSFGraph.h"

namespace itk
{
/**\class OSFGraphSource
 * \brief The superclass for all objects that generate an OSF graph object.
 * \date	12/9/2014
 * \author	Christian Bauer
 * The superclass for all objects that generate an OSF graph object. CONTINUE
 * Template parameters for class OSFGraphSource:
 *
 * - TOutputOSFGraph = The type of OSF graph produced by the object.
 *
 */
 
template <class TOutputOSFGraph>
class ITK_EXPORT OSFGraphSource : public ProcessObject
{
public:
  using Self = OSFGraphSource;
  using Superclass = ProcessObject;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  ITK_DISALLOW_COPY_AND_ASSIGN(OSFGraphSource);
  
  itkNewMacro( Self );
  itkTypeMacro( OSFGraphSource, ProcessObject );
  
  /** Some convenient type aliases. */
  //using DataObjectPointer = DataObject::Pointer ;
  using OutputOSFGraphType = TOutputOSFGraph;
  using OutputOSFGraphPointer = typename OutputOSFGraphType::Pointer;
  
  /** Get the mesh output of this process object.  */
  OutputOSFGraphType* GetOutput(void);
  OutputOSFGraphType* GetOutput(unsigned int idx);
  
  /** Set the OSF graph output of this process object. This call is slated
   * to be removed from ITK. You should GraftOutput() and possible
   * DataObject::DisconnectPipeline() to properly change the output. */
  using Superclass::SetOutput;
  void SetOutput(TOutputOSFGraph *output);
  virtual void GraftOutput(DataObject *output);
  virtual void GraftNthOutput(unsigned int idx, DataObject *output);
  using Superclass::MakeOutput;
  ProcessObject::DataObjectPointer MakeOutput(ProcessObject::DataObjectPointerArraySizeType idx) override;
  
  //virtual void Update();

protected:
  /** Constructor for use by New() method. */
  OSFGraphSource();
  ~OSFGraphSource() override = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;
  
  void GenerateInputRequestedRegion() override;
    
private:
  
}; // end class OSFGraphSource

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFGraphSource.txx"
#endif

#endif

