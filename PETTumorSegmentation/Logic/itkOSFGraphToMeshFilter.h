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

#ifndef __itkOSFGraphToMeshFilter_h
#define __itkOSFGraphToMeshFilter_h

#include "itkMeshSource.h"

namespace itk
{
/**\class OSFGraphToMeshFilter
 * \brief Converts a fully solved itk OSF graph into an itk Mesh.
 * \date	12/9/2014
 * \author	Chrsitian Bauer
 * Converts a fully solved itk OSF graph into an itk Mesh. CONTINUE
 * Template parameters for class OSFGraphToMeshFilter:
 *
 * - TInputOSFGraph = The graph type of the input to convert.
 * - TOutputMesh = The itk mesh type of the output after conversion.
 */
template <class TInputOSFGraph, class TOutputMesh>
class ITK_EXPORT OSFGraphToMeshFilter : public MeshSource<TOutputMesh>
{
public:
  /** Standard class typedefs. */
  typedef OSFGraphToMeshFilter     Self;
  typedef MeshSource<TOutputMesh>  Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  ITK_DISALLOW_COPY_AND_ASSIGN(OSFGraphToMeshFilter);
  
  itkNewMacro( Self );
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(OSFGraphToMeshFilter, MeshSource);
  
  /** Enum defining the type of surface to extract */
  typedef  enum { CurrentSurface,
                  InitialSurface,
                  InnermostSurface,
                  OutermostSurface
                                   } SurfaceType;
  
  /** Get/Set the type of surface to extract */
  itkSetMacro( SurfaceType, SurfaceType );
  itkGetMacro( SurfaceType, SurfaceType );

  /** Create a valid output. */
  typedef typename Superclass::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;
  using Superclass::MakeOutput;
  DataObject::Pointer MakeOutput(DataObjectPointerArraySizeType idx);

  /** Some Image related typedefs. */
  typedef TInputOSFGraph InputOSFGraphType;
  typedef typename InputOSFGraphType::ConstPointer InputOSFGraphConstPointer;

  /** Some Mesh related typedefs. */
  typedef TOutputMesh OutputMeshType;
  typedef typename OutputMeshType::Pointer OutputMeshPointer;

  /** Set the input image of this process object.  */
  using Superclass::SetInput;
  void SetInput(const InputOSFGraphType* input);

  /** Get the input image of this process object.  */
  const InputOSFGraphType* GetInput();

  /** Get the output Mesh of this process object.  */
  OutputMeshType* GetOutput(unsigned int idx);
  OutputMeshType* GetOutput(void) {return this->GetOutput(0);};

protected:
  OSFGraphToMeshFilter();
  ~OSFGraphToMeshFilter() override = default;
  virtual void PrintSelf(std::ostream& os, Indent indent) const override;
  
  void GenerateData() override;
  void GenerateOutputInformation(void);
  virtual void CopyInputOSFSurfaceToOutputMeshPoints(typename InputOSFGraphType::OSFSurface::ConstPointer osfSurface, OutputMeshPointer mesh);
  virtual void CopyInputOSFSurfaceToOutputMeshCells(typename InputOSFGraphType::OSFSurface::ConstPointer osfSurface, OutputMeshPointer mesh);
  
  SurfaceType m_SurfaceType;
  
private:
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFGraphToMeshFilter.txx"
#endif

#endif
