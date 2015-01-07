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

#ifndef _itkLOGISMOSOSFGraphSolverFilter_txx
#define _itkLOGISMOSOSFGraphSolverFilter_txx

#include "itkLOGISMOSOSFGraphSolverFilter.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::LOGISMOSOSFGraphSolverFilter() :
  m_MaxFlowGraph(NULL),
  m_FlowValue(0)
{
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::~LOGISMOSOSFGraphSolverFilter()
{
  if (m_MaxFlowGraph!=NULL)
  {
    delete m_MaxFlowGraph;
    m_MaxFlowGraph = NULL;
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::GenerateData()
{
  this->CopyInputOSFGraphToOutputOSFGraphSurfaces();
  this->CopyInputOSFGraphToOutputOSFGraphGraph();
  InputOSFGraphConstPointer input = this->GetInput();
  
  // build graph
  //typename InputOSFGraphType::GraphNodeIdentifier numNodes = input->GetNumberOfNodes();
  //typename InputOSFGraphType::GraphEdgeIdentifier numEdges = input->GetNumberOfEdges();
  //m_MaxFlowGraph = new MaxFlowGraphType(numNodes, numEdges);
  m_MaxFlowGraph = new MaxFlowGraphType();
  
  this->BuildMaxFlowGraphGraph();
  // solve max flow
  //m_FlowValue = m_MaxFlowGraph->maxflow();
  m_FlowValue = m_MaxFlowGraph->solve();
  
  // store result
  this->UpdateResult();
  delete m_MaxFlowGraph;
  m_MaxFlowGraph = NULL;
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::BuildMaxFlowGraphGraph()
{
  // note: we assume Boykov's max flow lib procudes the same node_id's we use

  // add nodes with terminal weights
  typedef typename InputOSFGraphType::GraphNodesContainer GraphNodeContainer;
  typename GraphNodeContainer::ConstPointer graphNodes = this->GetInput()->GetNodes();
  typename GraphNodeContainer::ConstIterator graphNodesItr = graphNodes->Begin();
  typename GraphNodeContainer::ConstIterator graphNodesEnd = graphNodes->End();
  
  //m_MaxFlowGraph->add_node( graphNodes->Size() );
  std::size_t id = m_MaxFlowGraph->add_nodes( graphNodes->Size() );

  while ( graphNodesItr!=graphNodesEnd )
  {
    typename InputOSFGraphType::GraphNodeIdentifier nodeId = graphNodesItr.Index();
    const typename InputOSFGraphType::GraphNode& node = graphNodesItr.Value();
    //m_MaxFlowGraph->add_tweights( nodeId, node.cap_source, node.cap_sink );
    m_MaxFlowGraph->add_st_edge( nodeId, node.cap_source, node.cap_sink );    
    ++graphNodesItr;
  }
  
  // alternative to above node creation for checking if nodeIds of boykovs and our library match
  //while ( graphNodesItr!=graphNodesEnd )
  //{
  //  unsigned int boykovNodeId = m_MaxFlowGraph->add_node( );
  //  typename InputOSFGraphType::GraphNodeIdentifier nodeId = graphNodesItr.Index();
  //  if (boykovNodeId!=nodeId)
  //    std::cout << "nodes do not match!" << std::endl;
  //  const typename InputOSFGraphType::GraphNode& node = graphNodesItr.Value();
  //  m_MaxFlowGraph->add_tweights( nodeId, node.cap_source, node.cap_sink );
  //  ++graphNodesItr;
  //}
  
  // add edges
  typedef typename InputOSFGraphType::GraphEdgesContainer GraphEdgesContainer;
  typename GraphEdgesContainer::ConstPointer graphEdges = this->GetInput()->GetEdges();
  typename GraphEdgesContainer::ConstIterator graphEdgesItr = graphEdges->Begin();
  typename GraphEdgesContainer::ConstIterator graphEdgesEnd = graphEdges->End();
  while ( graphEdgesItr!=graphEdgesEnd )
  {
    const typename InputOSFGraphType::GraphEdge& edge = graphEdgesItr.Value();
    //m_MaxFlowGraph->add_edge( edge.startNodeId, edge.endNodeId, edge.cap, edge.rev_cap );
    //m_MaxFlowGraph->add_edge( edge.startNodeId, edge.endNodeId, edge.cap );
    //m_MaxFlowGraph->add_edge( edge.endNodeId, edge.startNodeId, edge.rev_cap );
    m_MaxFlowGraph->add_edge( edge.startNodeId, edge.endNodeId, edge.cap, edge.rev_cap );
    ++graphEdgesItr;
  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::UpdateResult()
{
  // note: we assume Boykov's max flow lib procudes the same node_id's we use
  
  // note: instead of iterating through all nodes, we could do a binary search on the nodes associated with a column
  // this could give some speedup in case of many column positions
  
  if (m_MaxFlowGraph==NULL)
    return;
    
  InputOSFGraphConstPointer input = this->GetInput();
  OutputOSFGraphPointer output = this->GetOutput();
  
  // set all column positions to 0 first
  for (typename OutputOSFGraphType::SurfaceIdentifier surfaceId=0; surfaceId<output->GetNumberOfSurfaces(); surfaceId++)
  {
    typename OutputOSFGraphType::OSFSurfacePointer surface = output->GetSurface(surfaceId);
    for (typename OutputOSFGraphType::VertexIdentifier vertexId=0; vertexId<surface->GetNumberOfVertices(); vertexId++)
      surface->SetCurrentVertexPositionIdentifier(vertexId,0);
  }
  
  // update to higest column position still attached to the source
  std::size_t numNodes = m_MaxFlowGraph->get_node_cnt();
  for (std::size_t nodeId=0; nodeId<numNodes; nodeId++)
  {
    //typename MaxFlowGraphType::termtype terminal = m_MaxFlowGraph->what_segment(nodeId);
    //if (terminal==MaxFlowGraphType::SOURCE) // node included in the segmentation
    if (m_MaxFlowGraph->in_source_set(nodeId))
    {
      // update mesh position
      const typename InputOSFGraphType::GraphNode& node = input->GetNode( nodeId );
      typename OutputOSFGraphType::OSFSurfacePointer surface = output->GetSurface(node.surfaceId);
      typename OutputOSFGraphType::ColumnPositionIdentifier currentColumnPosition = surface->GetCurrentVertexPositionIdentifier(node.vertexId);
      if (node.positionId>currentColumnPosition)
        surface->SetCurrentVertexPositionIdentifier(node.vertexId, node.positionId);
    }
  }
  
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
LOGISMOSOSFGraphSolverFilter<TInputOSFGraph, TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif

