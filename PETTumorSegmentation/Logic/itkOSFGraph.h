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

#ifndef _itkOSFGraph_h
#define _itkOSFGraph_h

#include <itkObject.h>
#include "itkOSFSurface.h"

namespace itk
{
/**\class OSFGraph
 * \brief The class for holding the graph information.
 * \date	12/9/2014
 * \author	Chrsitian Bauer
 * ?
 * Template parameters for class OSFGraph:
 *
 * - TCostType = ?
 * - TOSFSurfaceType = ?
 */
template <typename TCostType, class TOSFSurfaceType=OSFSurface<float> >
class ITK_EXPORT OSFGraph : public DataObject
{
public:
  typedef OSFGraph Self;
  typedef DataObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  ITK_DISALLOW_COPY_AND_ASSIGN(OSFGraph);
  
  itkNewMacro( Self );
  itkTypeMacro( OSFGraph, DataObject );
  
  typedef unsigned long IdentifierType;

  //--------------------------------------------------------------
  // surfaces
  //--------------------------------------------------------------
  typedef TOSFSurfaceType OSFSurface;
  typedef typename OSFSurface::VertexIdentifier VertexIdentifier;
  typedef typename OSFSurface::ColumnPositionIdentifier ColumnPositionIdentifier;
  typedef IdentifierType SurfaceIdentifier;
  typedef typename OSFSurface::Pointer OSFSurfacePointer;
  typedef typename OSFSurface::ConstPointer OSFSurfaceConstPointer;
  typedef VectorContainer< SurfaceIdentifier, OSFSurfacePointer > SurfacesContainer;
  
  // access to surfaces
  SurfaceIdentifier GetNumberOfSurfaces() const;
  OSFSurface* GetSurface();
  const OSFSurface* GetSurface() const;
  OSFSurface* GetSurface(SurfaceIdentifier surfaceId);
  const OSFSurface* GetSurface(SurfaceIdentifier surfaceId) const;
  void SetSurface(OSFSurfacePointer surface) {return this->SetSurface(0, surface);};
  void SetSurface(SurfaceIdentifier surfaceId, OSFSurface* surface);
 
  //--------------------------------------------------------------
  // graph nodes and edges
  //--------------------------------------------------------------
  typedef TCostType GraphCosts;
  typedef IdentifierType GraphNodeIdentifier;
  typedef IdentifierType GraphEdgeIdentifier;
  
  class GraphNode
  {
    public:
      GraphNode() :
        surfaceId(0), vertexId(0), positionId(0), cap_source(0), cap_sink(0) {};
      GraphNode(SurfaceIdentifier surfaceId_, VertexIdentifier vertexId_, ColumnPositionIdentifier positionId_, GraphCosts cap_source_, GraphCosts cap_sink_) :
        surfaceId(surfaceId_), vertexId(vertexId_), positionId(positionId_), cap_source(cap_source_), cap_sink(cap_sink_) {};
      SurfaceIdentifier surfaceId;
      VertexIdentifier vertexId;
      ColumnPositionIdentifier positionId;
      GraphCosts cap_source;
      GraphCosts cap_sink;
  };
  
  class GraphEdge
  {
    public:
      GraphEdge() :
        startNodeId(0), endNodeId(0), cap(0), rev_cap(0) {};
      GraphEdge(GraphNodeIdentifier startNodeId_, GraphNodeIdentifier endNodeId_, GraphCosts cap_, GraphCosts rev_cap_) :
        startNodeId(startNodeId_), endNodeId(endNodeId_), cap(cap_), rev_cap(rev_cap_) {};
      GraphNodeIdentifier startNodeId;
      GraphNodeIdentifier endNodeId;
      GraphCosts cap;
      GraphCosts rev_cap;
  };
  
  typedef VectorContainer< GraphNodeIdentifier, GraphNode > GraphNodesContainer;
  typedef VectorContainer< GraphEdgeIdentifier, GraphEdge > GraphEdgesContainer;
  
  // access to nodes
  GraphNodeIdentifier GetNumberOfNodes() const;
  GraphNode& GetNode(GraphNodeIdentifier nodeId);
  const GraphNode& GetNode(GraphNodeIdentifier nodeId) const;
  void SetNode(GraphNodeIdentifier nodeId, const GraphNode& node);
  GraphNodesContainer* GetNodes();
  const GraphNodesContainer* GetNodes() const;
  void SetNodes(GraphNodesContainer* nodes);
  
  // access to edges
  GraphEdgeIdentifier GetNumberOfEdges() const;
  GraphEdge& GetEdge(GraphEdgeIdentifier edgeId);
  const GraphEdge& GetEdge(GraphEdgeIdentifier edgeId) const;
  void SetEdge(GraphEdgeIdentifier edgeId, const GraphEdge& edge);
  GraphEdgesContainer* GetEdges();
  const GraphEdgesContainer* GetEdges() const;
  void SetEdges(GraphEdgesContainer* edges);
  
  // utility function for fast lookup of graph nodes/edges associated with a SurfaceVertexColumnPosition
  // after graph construction (nodes/edges) the internally used lookup table has to be build first
  void BuildGraphNodeIdentifierLookupTable();
  GraphNodeIdentifier GetNodeIdentifer(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId) const;
  GraphNode& GetNode(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId);
  const GraphNode& GetNode(SurfaceIdentifier surfaceId, VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId) const;
  
  /** Type used to define Regions */
  typedef long RegionType;
  
  void Initialize(void) override;

  /** Get the maximum number of regions that this data can be
   * separated into. */
  itkGetConstMacro( MaximumNumberOfRegions, RegionType );
  
  /** Set the requested region from this data object to match the requested
   * region of the data object passed in as a parameter.  This method 
   * implements the API from DataObject. The data object parameter must be
   * castable to a PointSet. */
  //virtual void SetRequestedRegion(DataObject *data); // TODO: this causes troubles when creating python binding for c++ overloading
  
  /** Methods to manage streaming. */
  void UpdateOutputInformation() override;
  void SetRequestedRegionToLargestPossibleRegion() override;
  void CopyInformation(const DataObject *data) override;
  void Graft(const DataObject *data) override;
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
  bool VerifyRequestedRegion() override;
  
  /** Set/Get the Requested region */
  //virtual void SetRequestedRegion( const RegionType & region ); // TODO: this causes troubles when creating python binding for c++ overloading
  itkGetConstMacro( RequestedRegion, RegionType );

  /** Set/Get the Buffered region */
  virtual void SetBufferedRegion( const RegionType & region );
  itkGetConstMacro( BufferedRegion, RegionType );
  
protected:
  /** Constructor for use by New() method. */
  OSFGraph();
  ~OSFGraph() override = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;
  
  typename SurfacesContainer::Pointer m_SurfacesContainer;
  typename GraphNodesContainer::Pointer m_GraphNodesContainer;
  typename GraphEdgesContainer::Pointer m_GraphEdgesContainer;
  
  std::vector< std::vector< std::vector< GraphNodeIdentifier > > > m_GraphNodeIdentifierLookupTable; // todo: probably not optimal data structure but should be sufficient
  
  // If the RegionType is ITK_UNSTRUCTURED_REGION, then the following
  // variables represent the maximum number of region that the data
  // object can be broken into, which region out of how many is
  // currently in the buffered region, and the number of regions and
  // the specific region requested for the update. Data objects that
  // do not support any division of the data can simply leave the
  // MaximumNumberOfRegions as 1. The RequestedNumberOfRegions and
  // RequestedRegion are used to define the currently requested
  // region. The LargestPossibleRegion is always requested region = 0
  // and number of regions = 1;
  RegionType m_MaximumNumberOfRegions;
  RegionType m_NumberOfRegions;
  RegionType m_RequestedNumberOfRegions;
  RegionType m_BufferedRegion;
  RegionType m_RequestedRegion;
  
private:
  
}; // end class OSFGraph

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFGraph.txx"
#endif

#endif

