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

#ifndef _itkMeshToOSFGraphFilter_h
#define _itkMeshToOSFGraphFilter_h

#include "itkOSFGraphSource.h"

namespace itk
{
/**\class MeshToOSFGraphFilter
 * \brief Converts an ITK mesh into an OSF graph of just the outer points.
 * \date	12/9/2014
 * \author	Christian Bauer
 * Converts an ITK mesh into an OSF graph of just the outer points. CONTINUE
 * Template parameters for class MeshToOSFGraphFilter:
 *
 * - TInputOSFGraph = The graph type of the input to convert.
 * - TOutputMesh = The itk mesh type of the output after conversion.
 */
template <class TInputMesh, class TOutputOSFGraph>
class ITK_EXPORT MeshToOSFGraphFilter : public OSFGraphSource<TOutputOSFGraph>
{
public:
  typedef MeshToOSFGraphFilter Self;
  typedef OSFGraphSource<TOutputOSFGraph> Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro( Self );
  itkTypeMacro( MeshToOSFGraphFilter, OSFGraphSource );
  
  typedef TInputMesh InputMeshType;
  typedef typename InputMeshType::ConstPointer InputMeshConstPointer;
  
  typedef TOutputOSFGraph OutputOSFGraphType;
  typedef typename OutputOSFGraphType::Pointer OutputOSFGraphPointer;
    
  virtual void SetInput(unsigned int idx, const InputMeshType* input); // Set the input image of this process object.
  virtual const InputMeshType* GetInput(unsigned int idx); // Get the input image of this process object.
  virtual void SetInput( const InputMeshType* input) {this->SetInput(0, input);}; // Set the input image of this process object.
  virtual const InputMeshType* GetInput() {return this->GetInput(0);}; // Get the input image of this process object.
  
  OutputOSFGraphType* GetOutput(void); // Get the mesh output of this process object.
  virtual void GenerateOutputInformation(void); // Prepare the output
  virtual DataObject::Pointer MakeOutput(unsigned int idx); // create a valid output

protected:
  /** Constructor for use by New() method. */
  MeshToOSFGraphFilter();
  ~MeshToOSFGraphFilter() {};
  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  
  virtual void GenerateData();
  virtual void CopyInputMeshToOutputOSFSurfacePoints(InputMeshConstPointer mesh, typename OutputOSFGraphType::OSFSurface::Pointer osfSurface);
  virtual void CopyInputMeshToOutputOSFSurfaceCells(InputMeshConstPointer mesh, typename OutputOSFGraphType::OSFSurface::Pointer osfSurface);
    
private:
  MeshToOSFGraphFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
}; // end class MeshToOSFGraphFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMeshToOSFGraphFilter.txx"
#endif

#endif

