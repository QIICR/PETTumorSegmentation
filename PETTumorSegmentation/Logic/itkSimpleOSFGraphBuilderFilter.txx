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

#ifndef _itkSimpleOSFGraphBuilderFilter_txx
#define _itkSimpleOSFGraphBuilderFilter_txx

#include "itkSimpleOSFGraphBuilderFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNumericTraits.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::SimpleOSFGraphBuilderFilter() :
  m_Infinity( std::numeric_limits<typename TOutputOSFGraph::GraphCosts>::infinity() ),
  m_ColumnBasedNodeWeight( -1.0 ),
  m_SmoothnessConstraint( itk::NumericTraits<unsigned int>::max() ),
  m_SoftSmoothnessPenalty( 0 )
{
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::GenerateData()
{
  this->CopyInputOSFGraphToOutputOSFGraphSurfaces();
  OutputOSFGraphPointer output = this->GetOutput();
  
  // create nodes for columns
  for (SurfaceIdentifier surfaceId=0; surfaceId<output->GetNumberOfSurfaces(); surfaceId++)
  {
    typename OSFSurface::Pointer surface = output->GetSurface(surfaceId);
    for (VertexIdentifier vertexId=0; vertexId<surface->GetNumberOfVertices(); vertexId++)
      this->CreateNodesForColumn(surfaceId, vertexId);
  }
  output->BuildGraphNodeIdentifierLookupTable();
 
  // create intra-column arcs
  for (SurfaceIdentifier surfaceId=0; surfaceId<output->GetNumberOfSurfaces(); surfaceId++)
  {
    typename OSFSurface::Pointer surface = output->GetSurface(surfaceId);
    for (VertexIdentifier vertexId=0; vertexId<surface->GetNumberOfVertices(); vertexId++)
      this->CreateIntraColumnArcsForColumn(surfaceId, vertexId);
  }

  // create inter-column arcs
  for (SurfaceIdentifier surfaceId=0; surfaceId<output->GetNumberOfSurfaces(); surfaceId++)
  {
    typename OSFSurface::Pointer surface = output->GetSurface(surfaceId);
    surface->BuildNeighborLookupTable();
    for (VertexIdentifier vertexId=0; vertexId<surface->GetNumberOfVertices(); vertexId++)
      this->CreateInterColumnArcsForColumn(surfaceId, vertexId);
  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::CreateNodesForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId)
{
  typedef typename OutputOSFGraphType::GraphNode GraphNode;
  
  OutputOSFGraphPointer output = this->GetOutput();
  typename OSFSurface::ColumnCostsContainer::ConstPointer columnCosts = output->GetSurface(surfaceId)->GetColumnCosts(vertexId);
  typename OSFSurface::ColumnCostsContainer::ConstIterator columnCostsItr = columnCosts->Begin();
  typename OSFSurface::ColumnCostsContainer::ConstIterator columnCostsEnd = columnCosts->End();
  typename OutputOSFGraphType::GraphNodesContainer::Pointer graphNodes = output->GetNodes();
  typename OutputOSFGraphType::GraphNodeIdentifier startNodeIndex = graphNodes->Size();
  graphNodes->Reserve( graphNodes->Size()+columnCosts->Size() );
  
  if (columnCosts->Size()>0)
  {
    typename OutputOSFGraphType::GraphCosts weight = 0;
    typename OutputOSFGraphType::GraphCosts previousNodeCost = columnCostsItr.Value();
    graphNodes->SetElement( startNodeIndex++, GraphNode(surfaceId, vertexId, columnCostsItr->Index(), -m_ColumnBasedNodeWeight, 0) ); // set node of base to default value
    ++columnCostsItr;
    while (columnCostsItr!=columnCostsEnd)
    {
      weight = columnCostsItr.Value()-previousNodeCost;
      if (weight>0) // non-negative -> connect to t
        graphNodes->SetElement( startNodeIndex++, GraphNode(surfaceId, vertexId, columnCostsItr->Index(), 0.0, weight) );
      else // negative -> connect to s
        graphNodes->SetElement( startNodeIndex++, GraphNode(surfaceId, vertexId, columnCostsItr->Index(), -weight, 0.0) );
      previousNodeCost = columnCostsItr.Value();
      ++columnCostsItr;
    }
  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::CreateIntraColumnArcsForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId)
{
  //Edges along columns, from node to node, outside to inside, with infinite capacity
  //These relate the nodes on the column in the graph, allowing for selection of a single one
  typedef typename OutputOSFGraphType::GraphEdge GraphEdge;
  typedef typename OutputOSFGraphType::GraphNodeIdentifier GraphNodeIdentifier;
  
  OutputOSFGraphPointer output = this->GetOutput();
  typename OSFSurface::ColumnPositionIdentifier numColumnPositions = output->GetSurface(surfaceId)->GetNumberOfColumns(vertexId);
  typename OutputOSFGraphType::GraphEdgesContainer::Pointer graphEdges = output->GetEdges();
  typename OutputOSFGraphType::GraphEdgeIdentifier startEdgeIndex = graphEdges->Size();
  graphEdges->Reserve( graphEdges->Size()+numColumnPositions-1 );
  
  for (typename OSFSurface::ColumnPositionIdentifier columnPositionId=1; columnPositionId<numColumnPositions; columnPositionId++)
  {
    GraphNodeIdentifier startNodeId = output->GetNodeIdentifer(surfaceId, vertexId, columnPositionId);
    GraphNodeIdentifier endNodeId = output->GetNodeIdentifer(surfaceId, vertexId, columnPositionId-1);
    graphEdges->SetElement( startEdgeIndex++, GraphEdge( startNodeId, endNodeId, m_Infinity, 0) );
  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::CreateInterColumnArcsForColumn(SurfaceIdentifier surfaceId, VertexIdentifier vertexId)
{
  typedef typename OutputOSFGraphType::GraphEdge GraphEdge;
  typedef typename OutputOSFGraphType::GraphNodeIdentifier GraphNodeIdentifier;
  
  OutputOSFGraphPointer output = this->GetOutput();
  typename OSFSurface::ColumnPositionIdentifier numColumnPositions = output->GetSurface(surfaceId)->GetNumberOfColumns(vertexId);
  typename OSFSurface::VertexIdentifierContainer::ConstPointer neighbors = output->GetSurface(surfaceId)->GetNeighbors(vertexId);
  typename OutputOSFGraphType::GraphEdgesContainer::Pointer graphEdges = output->GetEdges();
  
  for (typename OSFSurface::VertexIdentifierContainer::ConstIterator neighborItr=neighbors->Begin(); neighborItr!=neighbors->End(); ++neighborItr)
  {
    // add hard smoothness constraints
    if (m_SmoothnessConstraint!=itk::NumericTraits<unsigned int>::max())
    {
      for (typename OSFSurface::ColumnPositionIdentifier columnPositionId=0; columnPositionId<numColumnPositions; columnPositionId++)
      {
        GraphNodeIdentifier startNodeId = output->GetNodeIdentifer(surfaceId, vertexId, columnPositionId);
        
        // note: computation of columnPositionId for the neighbor node should include the initialVertexPositionId
        GraphNodeIdentifier endNodeId = output->GetNodeIdentifer(surfaceId, neighborItr.Value(), typename OSFSurface::ColumnPositionIdentifier(std::max( (int)columnPositionId-(int)m_SmoothnessConstraint, 0)) );
        graphEdges->InsertElement( graphEdges->Size(), GraphEdge( startNodeId, endNodeId, m_Infinity, 0) );
      }
    }
    // add soft smoothness penalty term
    if (m_SoftSmoothnessPenalty>0.0 && neighborItr.Value()>vertexId)
    {
      for (typename OSFSurface::ColumnPositionIdentifier columnPositionId=0; columnPositionId<numColumnPositions; columnPositionId++)
      {
        GraphNodeIdentifier startNodeId = output->GetNodeIdentifer(surfaceId, vertexId, columnPositionId);
        
        // note: computation of columnPositionId for the neighbor node should include the initialVertexPositionId
        GraphNodeIdentifier endNodeId = output->GetNodeIdentifer(surfaceId, neighborItr.Value(), columnPositionId );
        graphEdges->InsertElement( graphEdges->Size(), GraphEdge( startNodeId, endNodeId, m_SoftSmoothnessPenalty, m_SoftSmoothnessPenalty) );
      }
    }
    

  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
SimpleOSFGraphBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif

