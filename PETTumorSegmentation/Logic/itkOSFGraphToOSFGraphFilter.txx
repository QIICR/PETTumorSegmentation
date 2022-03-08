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

#ifndef _itkOSFGraphToOSFGraphFilter_txx
#define _itkOSFGraphToOSFGraphFilter_txx

#include "itkOSFGraphToOSFGraphFilter.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::OSFGraphToOSFGraphFilter()
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::SetInput(unsigned int idx, const InputOSFGraphType* input)
{
  // process object is not const-correct, the const_cast is required here.
  this->ProcessObject::SetNthInput(idx, const_cast< TInputOSFGraph * >(input) );
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
const typename OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>::InputOSFGraphType*
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::GetInput(unsigned int idx) const
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }
  return dynamic_cast<const TInputOSFGraph*> (this->ProcessObject::GetInput(idx));
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
typename OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>::OutputOSFGraphType*
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::GetOutput(unsigned int idx)
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }
  return dynamic_cast<TOutputOSFGraph*> (this->ProcessObject::GetOutput(idx));
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::CopyInputOSFGraphToOutputOSFGraphSurfaces()
{
  InputOSFGraphConstPointer inputOSFGraph = this->GetInput();
  OutputOSFGraphPointer outputOSFGraph = this->GetOutput();
  for (typename OutputOSFGraphType::SurfaceIdentifier surfaceId=0; surfaceId<inputOSFGraph->GetNumberOfSurfaces(); surfaceId++)
  {
    OSFSurfaceConstPointer currentInputSurface = inputOSFGraph->GetSurface(surfaceId);
    OSFSurfacePointer currentOutputSurface = outputOSFGraph->GetSurface(surfaceId);
    this->CopyInputOSFGraphToOutputOSFGraphSurface( currentInputSurface, currentOutputSurface );
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::CopyInputOSFGraphToOutputOSFGraphSurface(OSFSurfaceConstPointer inputOSFSurface, OSFSurfacePointer outputOSFSurface)
{  
  // copy vertex information
  typename OSFSurface::VertexIdentifier numVertices = inputOSFSurface->GetNumberOfVertices();
  for (typename OSFSurface::VertexIdentifier vertexId=0; vertexId<numVertices; vertexId++)
  {
    // copy column point coordinates
    {
      using InputColumnCoordinatesContainerType = typename InputOSFGraphType::OSFSurface::ColumnCoordinatesContainer;
      typename InputColumnCoordinatesContainerType::ConstPointer inputPoints = inputOSFSurface->GetColumnCoordinates( vertexId );
      if (inputPoints)
      {
        typename InputColumnCoordinatesContainerType::ConstIterator inputItr = inputPoints->Begin();
        typename InputColumnCoordinatesContainerType::ConstIterator inputEnd = inputPoints->End();
        using OutputColumnCoordinatesContainerType = typename OutputOSFGraphType::OSFSurface::ColumnCoordinatesContainer;
        typename OutputColumnCoordinatesContainerType::Pointer outputPoints = OutputColumnCoordinatesContainerType::New();
        outputPoints->Reserve( inputPoints->Size() );
        typename OutputColumnCoordinatesContainerType::Iterator outputItr = outputPoints->Begin();
        while (inputItr!=inputEnd)
        {
          outputItr.Value() = inputItr.Value();
          ++inputItr; ++outputItr;
        }
        outputOSFSurface->SetColumnCoordinates( vertexId, outputPoints );
      }
    }
    
    // copy column point costs
    {
      using InputColumnCostsContainerType = typename InputOSFGraphType::OSFSurface::ColumnCostsContainer;
      typename InputColumnCostsContainerType::ConstPointer inputCosts = inputOSFSurface->GetColumnCosts( vertexId );
      if (inputCosts)
      {
        typename InputColumnCostsContainerType::ConstIterator inputItr = inputCosts->Begin();
        typename InputColumnCostsContainerType::ConstIterator inputEnd = inputCosts->End();
        using OutputColumnCostsContainerType = typename OutputOSFGraphType::OSFSurface::ColumnCostsContainer;
        typename OutputColumnCostsContainerType::Pointer outputCosts = OutputColumnCostsContainerType::New();
        outputCosts->Reserve( inputCosts->Size() );
        typename OutputColumnCostsContainerType::Iterator outputItr = outputCosts->Begin();
        while (inputItr!=inputEnd)
        {
          outputItr.Value() = inputItr.Value();
          ++inputItr; ++outputItr;
        }
        outputOSFSurface->SetColumnCosts( vertexId, outputCosts );
      }
    }
    
    outputOSFSurface->SetInitialVertexPositionIdentifier( vertexId, inputOSFSurface->GetInitialVertexPositionIdentifier(vertexId) );
    outputOSFSurface->SetCurrentVertexPositionIdentifier( vertexId, inputOSFSurface->GetCurrentVertexPositionIdentifier(vertexId) );
  }

  // copy cells 
  using OutputCellsContainer = typename OutputOSFGraphType::OSFSurface::CellsContainer;
  using InputCellsContainer = typename InputOSFGraphType::OSFSurface::CellsContainer;
  using CellAutoPointer = typename OutputOSFGraphType::OSFSurface::CellAutoPointer;

  typename OutputCellsContainer::Pointer outputCells = OutputCellsContainer::New();
  const InputCellsContainer * inputCells = inputOSFSurface->GetCells();

  if( inputCells )
    {
    outputCells->Reserve( inputCells->Size() );

    typename InputCellsContainer::ConstIterator inputItr = inputCells->Begin();
    typename InputCellsContainer::ConstIterator inputEnd = inputCells->End();
    typename OutputCellsContainer::Iterator outputItr = outputCells->Begin();

    CellAutoPointer clone;

    while( inputItr != inputEnd )
      {
      inputItr.Value()->MakeCopy( clone );
      outputItr.Value() = clone.ReleaseOwnership();

      ++inputItr;
      ++outputItr;
      }

    outputOSFSurface->SetCells( outputCells );
    }
}
//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::CopyInputOSFGraphToOutputOSFGraphGraph()
{
  // copy nodes
  using GraphNodesContainer = typename OutputOSFGraphType::GraphNodesContainer;
  typename GraphNodesContainer::ConstPointer inputGraphNodes = this->GetInput()->GetNodes();
  typename GraphNodesContainer::Pointer outputGraphNodes = GraphNodesContainer::New();
  
  if (inputGraphNodes)
  {
    outputGraphNodes->Reserve( inputGraphNodes->Size() );
    typename GraphNodesContainer::ConstIterator inputItr = inputGraphNodes->Begin();
    typename GraphNodesContainer::ConstIterator inputEnd = inputGraphNodes->End();
    typename GraphNodesContainer::Iterator outputItr = outputGraphNodes->Begin();
    while( inputItr != inputEnd )
    {
      outputItr.Value() = inputItr.Value();
      ++inputItr;
      ++outputItr;
    }
    this->GetOutput()->SetNodes( outputGraphNodes );
  }

  // copy edges
  using GraphEdgesContainer = typename OutputOSFGraphType::GraphEdgesContainer;
  typename GraphEdgesContainer::ConstPointer inputGraphEdges = this->GetInput()->GetEdges();
  typename GraphEdgesContainer::Pointer outputGraphEdges = GraphEdgesContainer::New();
  
  if (inputGraphEdges)
  {
    outputGraphEdges->Reserve( inputGraphEdges->Size() );
    typename GraphEdgesContainer::ConstIterator inputItr = inputGraphEdges->Begin();
    typename GraphEdgesContainer::ConstIterator inputEnd = inputGraphEdges->End();
    typename GraphEdgesContainer::Iterator outputItr = outputGraphEdges->Begin();
    while( inputItr != inputEnd )
    {
      outputItr.Value() = inputItr.Value();
      ++inputItr;
      ++outputItr;
    }
    this->GetOutput()->SetEdges( outputGraphEdges );
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
OSFGraphToOSFGraphFilter<TInputOSFGraph, TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif

