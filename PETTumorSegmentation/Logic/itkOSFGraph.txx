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

#ifndef _itkOSFGraph_txx
#define _itkOSFGraph_txx

#include "itkOSFGraph.h"

namespace itk
{

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
OSFGraph<TCostType, TSurfaceMeshTraits >
::OSFGraph()
{
  m_SurfacesContainer = SurfacesContainer::New();
  m_GraphNodesContainer = GraphNodesContainer::New();
  m_GraphEdgesContainer = GraphEdgesContainer::New();

  // If we used unstructured regions instead of structured regions, then
  // assume this object was created by the user and this is region 0 of
  // 1 region.
  m_MaximumNumberOfRegions = 1;
  m_NumberOfRegions = 1;
  m_BufferedRegion  = -1;
  m_RequestedNumberOfRegions = 0;
  m_RequestedRegion = -1;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::SurfaceIdentifier
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNumberOfSurfaces() const
{
  return m_SurfacesContainer->Size();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::OSFSurface*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetSurface()
{
  return this->GetSurface(0);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::OSFSurface*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetSurface() const
{
  return this->GetSurface(0);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::OSFSurface*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetSurface(SurfaceIdentifier surfaceId)
{
  if ( !m_SurfacesContainer->IndexExists(surfaceId) || !m_SurfacesContainer->GetElement(surfaceId) )
    {
      m_SurfacesContainer->InsertElement( surfaceId, OSFSurface::New() ); // create container if it does not exists yet
    }
  return m_SurfacesContainer->GetElement( surfaceId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::OSFSurface*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetSurface(SurfaceIdentifier surfaceId) const
{
  return m_SurfacesContainer->GetElement( surfaceId ).GetPointer();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetSurface(SurfaceIdentifier surfaceId, OSFSurface* surface)
{
  if ( !m_SurfacesContainer->IndexExists(surfaceId) )
  {
    m_SurfacesContainer->InsertElement( surfaceId, surface );
    this->Modified();
  }
  else if (m_SurfacesContainer->GetElement(surfaceId)!=surface)
  {
    m_SurfacesContainer->SetElement( surfaceId, surface );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNodeIdentifier
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNumberOfNodes() const
{
  return m_GraphNodesContainer->Size();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNode&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNode(GraphNodeIdentifier nodeId)
{
  if ( !m_GraphNodesContainer->IndexExists(nodeId) )
    {
      m_GraphNodesContainer->InsertElement( nodeId, GraphNode() ); // create container if it does not exists yet
    }
  return m_GraphNodesContainer->ElementAt( nodeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNode&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNode(GraphNodeIdentifier nodeId) const
{
  if ( !m_GraphNodesContainer->IndexExists(nodeId) )
    {
      m_GraphNodesContainer->InsertElement( nodeId, GraphNode() ); // create container if it does not exists yet
    }
  return m_GraphNodesContainer->ElementAt( nodeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetNode(GraphNodeIdentifier nodeId, const GraphNode& node)
{
  if ( !m_GraphNodesContainer->IndexExists(nodeId) )
  {
    m_GraphNodesContainer->InsertElement( nodeId, node );
    this->Modified();
  }
  else
  {
    m_GraphNodesContainer->SetElement( nodeId, node );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNodesContainer*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNodes()
{
  return m_GraphNodesContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNodesContainer*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNodes() const
{
  return m_GraphNodesContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetNodes(GraphNodesContainer* nodes)
{
  if (m_GraphNodesContainer!=nodes)
  {
    m_GraphNodesContainer = nodes;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphEdgeIdentifier
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNumberOfEdges() const
{
  return m_GraphEdgesContainer->Size();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphEdge&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetEdge(GraphEdgeIdentifier edgeId)
{
  if ( !m_GraphEdgesContainer->IndexExists(edgeId) )
    {
      m_GraphEdgesContainer->InsertElement( edgeId, GraphEdge() ); // create container if it does not exists yet
    }
  return m_GraphEdgesContainer->ElementAt( edgeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphEdge&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetEdge(GraphEdgeIdentifier edgeId) const
{
  if ( !m_GraphEdgesContainer->IndexExists(edgeId) )
    {
      m_GraphEdgesContainer->InsertElement( edgeId, GraphEdge() ); // create container if it does not exists yet
    }
  return m_GraphEdgesContainer->ElementAt( edgeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetEdge(GraphEdgeIdentifier edgeId, const GraphEdge& edge)
{
  if ( !m_GraphEdgesContainer->IndexExists(edgeId) )
  {
    m_GraphEdgesContainer->InsertElement( edgeId, edge );
    this->Modified();
  }
  else
  {
    m_GraphEdgesContainer->SetElement( edgeId, edge );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphEdgesContainer*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetEdges()
{
  return m_GraphEdgesContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphEdgesContainer*
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetEdges() const
{
  return m_GraphEdgesContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetEdges(GraphEdgesContainer* edges)
{
  if (m_GraphEdgesContainer!=edges)
  {
    m_GraphEdgesContainer = edges;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::BuildGraphNodeIdentifierLookupTable()
{
  // note: because of the type of the datastrucure for the lookup table and it's construction a lot of copying and reallocation may occur.
  // -> reimplement the lookup table if really slow in practice

  for (typename GraphNodesContainer::ConstIterator nodeIt=m_GraphNodesContainer->Begin(); nodeIt!=m_GraphNodesContainer->End(); nodeIt++)
  {
    GraphNodeIdentifier nodeId = nodeIt.Index();
    const GraphNode& node = nodeIt.Value();
    SurfaceIdentifier surfaceId = node.surfaceId;
    VertexIdentifier vertexId = node.vertexId;
    ColumnPositionIdentifier positionId = node.positionId;

    //std::cout << nodeId << " " << surfaceId << " " << vertexId <<  " " << positionId << std::endl;

    if (m_GraphNodeIdentifierLookupTable.size()<surfaceId+1)
      m_GraphNodeIdentifierLookupTable.resize(surfaceId+1);
    if (m_GraphNodeIdentifierLookupTable[surfaceId].size()<vertexId+1)
      m_GraphNodeIdentifierLookupTable[surfaceId].resize(vertexId+1);
    if (m_GraphNodeIdentifierLookupTable[surfaceId][vertexId].size()<positionId+1)
      m_GraphNodeIdentifierLookupTable[surfaceId][vertexId].resize(positionId+1);
    m_GraphNodeIdentifierLookupTable[surfaceId][vertexId][positionId] = nodeId;
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNodeIdentifier
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNodeIdentifer(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId) const
{
  if (surfaceId+1>m_GraphNodeIdentifierLookupTable.size())
    return 0;
  if (vertexId+1>m_GraphNodeIdentifierLookupTable[surfaceId].size())
    return 0;
  if (columnPositionId+1>m_GraphNodeIdentifierLookupTable[surfaceId][vertexId].size())
    return 0;
  return m_GraphNodeIdentifierLookupTable[surfaceId][vertexId][columnPositionId];
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNode&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNode(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId)
{
  GraphNodeIdentifier graphNodeId = this->GetNodeIdentifer( surfaceId, vertexId, columnPositionId );
  return this->GetNode( graphNodeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFGraph<TCostType, TSurfaceMeshTraits >::GraphNode&
OSFGraph<TCostType, TSurfaceMeshTraits >
::GetNode(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId) const
{
  GraphNodeIdentifier graphNodeId = this->GetNodeIdentifer( surfaceId, vertexId, columnPositionId );
  return this->GetNode( graphNodeId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::UpdateOutputInformation()
{
  if (this->GetSource())
    {
    this->GetSource()->UpdateOutputInformation();
    }

  // Now we should know what our largest possible region is. If our
  // requested region was not set yet, (or has been set to something
  // invalid - with no data in it ) then set it to the largest
  // possible region.
  if ( m_RequestedRegion == -1 && m_RequestedNumberOfRegions == 0 )
    {
    this->SetRequestedRegionToLargestPossibleRegion();
    }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedNumberOfRegions     = 1;
  m_RequestedRegion           = 0;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::CopyInformation(const DataObject *data)
{
  const OSFGraph * osfGraph = nullptr;

  try
    {
    osfGraph = dynamic_cast<const OSFGraph*>(data);
    }
  catch( ... )
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::OSFGraph::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(OSFGraph*).name() );
    }

  if ( !osfGraph )
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::OSFGraph::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(OSFGraph*).name() );
    }

  m_MaximumNumberOfRegions = osfGraph->GetMaximumNumberOfRegions();
  m_NumberOfRegions = osfGraph->m_NumberOfRegions;
  m_RequestedNumberOfRegions = osfGraph->m_RequestedNumberOfRegions;
  m_BufferedRegion  = osfGraph->m_BufferedRegion;
  m_RequestedRegion = osfGraph->m_RequestedRegion;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::Graft(const DataObject *data)
{
  // Copy Meta Data
  this->CopyInformation( data );

  const Self * osfGraph = nullptr;

  try
    {
    osfGraph = dynamic_cast<const Self*>(data);
    }
  catch( ... )
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::OSFGraph::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(Self*).name() );
    }

  if ( !osfGraph )
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::OSFGraph::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(Self*).name() );
    }

  for (SurfaceIdentifier surfaceId=0; surfaceId<osfGraph->m_SurfacesContainer->Size(); surfaceId++)
    this->SetSurface( surfaceId, osfGraph->m_SurfacesContainer->ElementAt(surfaceId) );
  this->SetNodes( osfGraph->m_GraphNodesContainer );
  this->SetEdges( osfGraph->m_GraphEdgesContainer );
}

/*
//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetRequestedRegion(DataObject *data)
{
  Self *osfGraph;

  osfGraph = dynamic_cast<Self*>(data);

  if ( osfGraph )
    {
    // only copy the RequestedRegion if the parameter is another OSFGraph
    m_RequestedRegion = osfGraph->m_RequestedRegion;
    m_RequestedNumberOfRegions = osfGraph->m_RequestedNumberOfRegions;
    }
}
*/
/*
//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetRequestedRegion(const RegionType &region)
{
  if (m_RequestedRegion != region)
    {
    m_RequestedRegion = region;
    }
}
*/

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::SetBufferedRegion(const RegionType &region)
{
  if (m_BufferedRegion != region)
    {
    m_BufferedRegion = region;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
bool
OSFGraph<TCostType, TSurfaceMeshTraits >
::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if ( m_RequestedRegion != m_BufferedRegion ||
       m_RequestedNumberOfRegions != m_NumberOfRegions )
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
bool
OSFGraph<TCostType, TSurfaceMeshTraits >
::VerifyRequestedRegion()
{
  bool retval = true;

  // Are we asking for more regions than we can get?
  if ( m_RequestedNumberOfRegions > m_MaximumNumberOfRegions )
    {
    itkExceptionMacro( << "Cannot break object into " <<
                       m_RequestedNumberOfRegions << ". The limit is " <<
                       m_MaximumNumberOfRegions );
    }

  if ( m_RequestedRegion >= m_RequestedNumberOfRegions ||
       m_RequestedRegion < 0 )
    {
    itkExceptionMacro( << "Invalid update region " << m_RequestedRegion
                       << ". Must be between 0 and "
                       << m_RequestedNumberOfRegions - 1);
    }

  return retval;
}

//----------------------------------------------------------------------------
/**
 * Restore the PointSet to its initial state.  Useful for data pipeline updates
 * without memory re-allocation.
 */
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::Initialize(void)
{
return;
  Superclass::Initialize();

  m_SurfacesContainer = SurfacesContainer::New();
  m_GraphNodesContainer = GraphNodesContainer::New();
  m_GraphEdgesContainer = GraphEdgesContainer::New();

}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFGraph<TCostType, TSurfaceMeshTraits >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  // todo: implement
}

} // namespace

#endif
