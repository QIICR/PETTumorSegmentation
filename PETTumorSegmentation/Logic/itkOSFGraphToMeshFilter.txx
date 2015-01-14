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

#ifndef __itkOSFGraphToMeshFilter_txx
#define __itkOSFGraphToMeshFilter_txx

#include "itkOSFGraphToMeshFilter.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::OSFGraphToMeshFilter() :
  m_SurfaceType(CurrentSurface)
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);

  OutputMeshPointer output
    = dynamic_cast<OutputMeshType*>(this->MakeOutput(0).GetPointer()); 

  this->ProcessObject::SetNumberOfRequiredOutputs(1);
  this->ProcessObject::SetNthOutput(0, output.GetPointer());

}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::~OSFGraphToMeshFilter()
{
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
DataObject::Pointer
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::MakeOutput(DataObjectPointerArraySizeType)
{
  OutputMeshPointer outputMesh = OutputMeshType::New();
  return dynamic_cast< DataObject *>( outputMesh.GetPointer() );
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::SetInput(const InputOSFGraphType* input)
{
  // process object is not const-correct, the const_cast is required here.
  this->ProcessObject::SetNthInput(0, const_cast< InputOSFGraphType * >(input) );
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
const typename OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>::InputOSFGraphType*
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::GetInput() 
{
  return dynamic_cast<const InputOSFGraphType*> (this->ProcessObject::GetInput(0));
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
typename OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>::OutputMeshType *
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::GetOutput(unsigned int idx) 
{
  return dynamic_cast<OutputMeshType*>
    (this->ProcessObject::GetOutput(idx));
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::GenerateData()
{
  InputOSFGraphConstPointer inputOSFGraph = this->GetInput();
  for (typename InputOSFGraphType::SurfaceIdentifier surfaceId=0; surfaceId<inputOSFGraph->GetNumberOfSurfaces(); surfaceId++)
  {
    OutputMeshPointer currentOutputSurface = this->GetOutput(surfaceId);
    // todo: we have to create the output meshes first in case we have multiple. One is already created per default in the constructor
    typename InputOSFGraphType::OSFSurface::ConstPointer currentInputSurface = inputOSFGraph->GetSurface(surfaceId);
    this->CopyInputOSFSurfaceToOutputMeshPoints(currentInputSurface, currentOutputSurface);
    this->CopyInputOSFSurfaceToOutputMeshCells(currentInputSurface, currentOutputSurface);
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::GenerateOutputInformation()
{
  // todo: do we have to do something here?
  // todo: does this actually create addititional output meshes depending on the number of surfaces in the input graph  
  for (typename InputOSFGraphType::SurfaceIdentifier surfaceId=this->GetNumberOfOutputs(); surfaceId<this->GetInput()->GetNumberOfSurfaces(); surfaceId++)
  {
    OutputMeshPointer output = dynamic_cast<OutputMeshType*>(this->MakeOutput(surfaceId).GetPointer());
    this->ProcessObject::SetNumberOfRequiredOutputs(this->GetInput()->GetNumberOfSurfaces());
    this->ProcessObject::SetNthOutput(surfaceId, output.GetPointer());
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::CopyInputOSFSurfaceToOutputMeshPoints(typename InputOSFGraphType::OSFSurface::ConstPointer osfSurface, OutputMeshPointer mesh)
{
  typedef typename OutputMeshType::PointType PointType;
  typename OutputMeshType::PointsContainerPointer points = OutputMeshType::PointsContainer::New();
  points->Reserve( osfSurface->GetNumberOfVertices() );
  typename OutputMeshType::PointsContainer::Iterator pointIterator = points->Begin();
  typename OutputMeshType::PointsContainer::Iterator pointEnd = points->End();
  
  while ( pointIterator != pointEnd )
  {
    typename InputOSFGraphType::OSFSurface::VertexIdentifier vertexId = pointIterator.Index();
    typename InputOSFGraphType::OSFSurface::ColumnPositionIdentifier columnPositionId = 0;
    switch(m_SurfaceType)
    {
      case CurrentSurface: columnPositionId = osfSurface->GetCurrentVertexPositionIdentifier(vertexId); break;
      case InitialSurface: columnPositionId = osfSurface->GetInitialVertexPositionIdentifier(vertexId); break;
      case InnermostSurface: columnPositionId = 0; break;
      case OutermostSurface: columnPositionId = (osfSurface->GetNumberOfColumns(vertexId)>0) ? osfSurface->GetNumberOfColumns(vertexId) : 0; break;
      default: columnPositionId = osfSurface->GetCurrentVertexPositionIdentifier(vertexId);
    }
    pointIterator.Value() = osfSurface->GetColumnCoordinates(vertexId)->ElementAt(columnPositionId);
    ++pointIterator;
  }
  
  mesh->SetPoints(points);
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::CopyInputOSFSurfaceToOutputMeshCells(typename InputOSFGraphType::OSFSurface::ConstPointer osfSurface, OutputMeshPointer mesh)
{
  typedef typename InputOSFGraphType::OSFSurface::CellsContainer InputCellsContainer;
  typedef typename TOutputMesh::CellsContainer OutputCellsContainer;
  typedef typename InputOSFGraphType::OSFSurface::CellAutoPointer CellAutoPointer;

  mesh->SetCellsAllocationMethod( OutputMeshType::CellsAllocatedDynamicallyCellByCell );

  typename OutputCellsContainer::Pointer outputCells = OutputCellsContainer::New();
  const InputCellsContainer * inputCells = osfSurface->GetCells();

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

    mesh->SetCells( outputCells );
    }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputMesh>
void 
OSFGraphToMeshFilter<TInputOSFGraph,TOutputMesh>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // end namespace itk

#endif
