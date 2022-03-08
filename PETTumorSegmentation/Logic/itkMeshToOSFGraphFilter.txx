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

#ifndef _itkMeshToOSFGraphFilter_txx
#define _itkMeshToOSFGraphFilter_txx

#include "itkMeshToOSFGraphFilter.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::MeshToOSFGraphFilter()
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);

  OutputOSFGraphPointer output
    = dynamic_cast<OutputOSFGraphType*>(this->MakeOutput(0).GetPointer()); 

  this->ProcessObject::SetNumberOfRequiredOutputs(1);
  this->ProcessObject::SetNthOutput(0, output.GetPointer());

}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::SetInput(unsigned int idx, const InputMeshType* input)
{
  // process object is not const-correct, the const_cast
  // is required here.
  this->ProcessObject::SetNthInput(idx, const_cast< InputMeshType * >(input) );
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
const typename MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>::InputMeshType*
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::GetInput(unsigned int idx)
{
  return dynamic_cast<const InputMeshType*> (this->ProcessObject::GetInput(idx));
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
typename MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>::OutputOSFGraphType*
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::GetOutput()
{
  return dynamic_cast<TOutputOSFGraph*> (this->ProcessObject::GetOutput(0));
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::GenerateOutputInformation()
{
  // todo: do we have to do something here?
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
DataObject::Pointer
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::MakeOutput(DataObjectPointerArraySizeType)
{
  OutputOSFGraphPointer outputOSFGraph = OutputOSFGraphType::New();
  return dynamic_cast< DataObject *>( outputOSFGraph.GetPointer() );
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::GenerateData()
{
  OutputOSFGraphPointer outputOSFGraph = this->GetOutput();
  for (typename OutputOSFGraphType::SurfaceIdentifier surfaceId=0; surfaceId<this->GetNumberOfInputs(); surfaceId++)
  {
    InputMeshConstPointer currentInputSurface = this->GetInput(surfaceId);
    typename OutputOSFGraphType::OSFSurface::Pointer currentOutputSurface = outputOSFGraph->GetSurface(surfaceId);
    this->CopyInputMeshToOutputOSFSurfacePoints(currentInputSurface,currentOutputSurface);
    this->CopyInputMeshToOutputOSFSurfaceCells(currentInputSurface,currentOutputSurface);
  }
  
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::CopyInputMeshToOutputOSFSurfacePoints(InputMeshConstPointer mesh, typename OutputOSFGraphType::OSFSurface::Pointer osfSurface)
{
  using PointIterator = typename TInputMesh::PointsContainer::ConstIterator;
  using PointType = typename TInputMesh::PointType;
  PointIterator pointIterator = mesh->GetPoints()->Begin();
  PointIterator pointEnd = mesh->GetPoints()->End();

  while( pointIterator != pointEnd )
    {
    typename OutputOSFGraphType::OSFSurface::VertexIdentifier vertexId = pointIterator.Index();
    PointType point = pointIterator.Value();
    
    using ColumnCoordinatesContainerType = typename OutputOSFGraphType::OSFSurface::ColumnCoordinatesContainer;
    typename ColumnCoordinatesContainerType::Pointer columnCoordinatesContainer = ColumnCoordinatesContainerType::New();
    columnCoordinatesContainer->InsertElement(0, point );
    osfSurface->SetColumnCoordinates(vertexId, columnCoordinatesContainer);
    using ColumnCostsContainerType = typename OutputOSFGraphType::OSFSurface::ColumnCostsContainer;
    typename ColumnCostsContainerType::Pointer columnCostsContainer = ColumnCostsContainerType::New();
    columnCostsContainer->InsertElement(0, 0.0 );
    osfSurface->SetColumnCosts(vertexId,columnCostsContainer);
    osfSurface->SetInitialVertexPositionIdentifier(vertexId,0);
    osfSurface->SetCurrentVertexPositionIdentifier(vertexId,0);
    
    pointIterator++;
    }
}

//----------------------------------------------------------------------------
template <class TInputMesh, class TOutputOSFGraph>
void
MeshToOSFGraphFilter<TInputMesh, TOutputOSFGraph>
::CopyInputMeshToOutputOSFSurfaceCells(InputMeshConstPointer mesh, typename OutputOSFGraphType::OSFSurface::Pointer osfSurface)
{
  using OutputCellsContainer = typename OutputOSFGraphType::OSFSurface::CellsContainer;
  using InputCellsContainer = typename TInputMesh::CellsContainer;
  using CellAutoPointer = typename OutputOSFGraphType::OSFSurface::CellAutoPointer;

  //outputMesh->SetCellsAllocationMethod( OutputMeshType::CellsAllocatedDynamicallyCellByCell );

  typename OutputCellsContainer::Pointer outputCells = OutputCellsContainer::New();
  const InputCellsContainer * inputCells = mesh->GetCells();

  if( inputCells )
    {
    outputCells->Reserve( inputCells->Size() );

    typename InputCellsContainer::ConstIterator inputItr = inputCells->Begin();
    typename InputCellsContainer::ConstIterator inputEnd = inputCells->End();

    typename OutputCellsContainer::Iterator outputItr = outputCells->Begin();

    CellAutoPointer clone;

    while( inputItr != inputEnd )
      {
//      outputItr.Value() = inputItr.Value();
      // note: this comment is from itkMeshToMeshFilter // BUG: FIXME: Here we are copying a pointer, which is a mistake. What we should do is to clone the cell.
      inputItr.Value()->MakeCopy( clone );
      outputItr.Value() = clone.ReleaseOwnership();

      ++inputItr;
      ++outputItr;
      }

    osfSurface->SetCells( outputCells );
    }
}

} // namespace

#endif

