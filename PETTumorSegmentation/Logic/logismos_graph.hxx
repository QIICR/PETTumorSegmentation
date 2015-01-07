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

#ifndef _LOGISMOS_graph_hxx_
#define _LOGISMOS_graph_hxx_

#include "logismos_chunk_list.hxx"
#include <queue>
#include <iostream>
#include <string>

namespace LOGISMOS{

/// \brief Data structure for graph designed for BK's maxflow algorithm.
///
/// _DataChunkSize is the size of a chunk (see chunk_list.hxx) used to store
/// graph nodes or edges, using large value for large graph can improve the performance.
/// _PtrChunkSize is the size of a chunk used to store pointers to edges associated with
/// a node, should be similar to the number of such edges.
template <typename _Cap, std::size_t _DataChunkSize=1024, std::size_t _PtrChunkSize=32>
class graph
{
  struct node;
  struct edge;
  
  typedef chunk_list<node, _DataChunkSize>  node_cont_type;     ///< for node container
  typedef chunk_list<edge, _DataChunkSize>  edge_cont_type;     ///< for edge container
  typedef std::deque<node*>                 node_p_queue_type;  ///< for FIFO of node pointers
  typedef chunk_list<edge*,_PtrChunkSize>   edge_p_cont_type;   ///< for 'array' of edge pointers
  
  /// \brief Data structure for a graph node
  struct node{
    edge_p_cont_type* m_out_edges;  ///< outgoing edges, tail=this node
    edge*             m_par_edge;   ///< parent edge in the search tree. 0: no parent, 1: terminal, 2: orphan
    
    _Cap              m_rcap;       ///< residual capacity, positive for source->node, negative for node->sink
    unsigned int      m_dist;       ///< distance (number of edges) to terminal node (source or sink)
    unsigned int      m_time;       ///< a time stamp indicates when m_dist is modified
    unsigned char     m_tag;        ///< several bitwise tags, [na, na, na, na, changed, marked, active, sink]
    
    /// \brief Constructor, probably will not be called by other member functions of the graph class
    node() : m_par_edge(0), m_rcap(0), m_dist(0), m_time(0), m_tag(0)
    { 
      m_out_edges = new edge_p_cont_type(); 
    }
    
    /// \brief Destructor.
    ~node(){  delete m_out_edges; m_out_edges = 0;}
    
    // quick ways to determine and set special status of the node and its parent edge.
    
    inline bool is_changed(){ return (m_tag&0x08)!=0; }
    inline bool is_marked(){  return (m_tag&0x04)!=0; }
    inline bool is_active(){  return (m_tag&0x02)!=0; }
    inline bool is_sink(){    return (m_tag&0x01)!=0; }
    
    inline bool has_parent(){   return m_par_edge!=0; }
    inline bool is_terminal(){  return m_par_edge==reinterpret_cast<edge*>(1);  }
    inline bool is_orphan(){    return m_par_edge==reinterpret_cast<edge*>(2);  }
    
    inline void set_changed(bool v){  m_tag = (v) ? (m_tag | 0x08) : (m_tag & ~0x08); }
    inline void set_marked(bool v){   m_tag = (v) ? (m_tag | 0x04) : (m_tag & ~0x04); }
    inline void set_active(bool v){   m_tag = (v) ? (m_tag | 0x02) : (m_tag & ~0x02); }
    inline void set_sink(bool v){     m_tag = (v) ? (m_tag | 0x01) : (m_tag & ~0x01); }
    
    inline void set_terminal(){ m_par_edge=reinterpret_cast<edge*>(1);  }
    inline void set_orphan(){   m_par_edge=reinterpret_cast<edge*>(2);  }
  };

  /// brief Data structure for a directed edge
  ///
  /// The directed edge starts from a tail node and ends at a head node with a given capacity.
  struct edge{
    node* m_head;   ///< node the edge points to
    _Cap  m_rcap;   ///< residual capacity of the edge
    edge* m_sister; ///< corresponding edge with opposite direction in the residual graph
  };
  
private:
  node_cont_type    m_nodes;          ///< all the graph nodes
  edge_cont_type    m_edges;          ///< all the graph edges
  node_p_queue_type m_active_nodes;   ///< a queue (FIFO) for active nodes
  node_p_queue_type m_orphan_nodes;   ///< a queue (FIFO) for orphan nodes
  unsigned int      m_clock;          ///< a global clock provides time stamp for all nodes
  _Cap              m_flow;           ///< total flow in the graph

  /// \brief Set node as active and add it to active node queue.
  inline void activate(node* p_node)
  {
    assert(!p_node);
    if(p_node->is_active() == false){
      p_node->set_active(true);
      m_active_nodes.push_back(p_node);
    } 
  }
  
  /// \brief Mark given node as orphan and add it to the orphan queue.
  inline void mark_orphan(node* p_node)
  {
    if(p_node->is_orphan() == false){
      p_node->set_orphan();
      m_orphan_nodes.push_back(p_node);
    }
  }
  
  /////////////////////////////////////////////////////////
  
  /// \brief Grow search trees from the given active node.
  /// 
  /// \return The edge that connects source and sink search trees, zero if not found.
  edge* grow_active_node(node* node_i);
  
  /// \brief Augment the path found by grow_active_node().
  void augment_path(edge* mid_edge);
  
  /// \brief Adopt the orphan node.
  void adopt_orphan(node* node_i);
  
  /////////////////////////////////////////////////////////
  
public:

  /// \brief Constructor. Create a empty graph
  graph() : m_clock(0), m_flow(0){  }
  
  /// \brief Destructor.
  ~graph(){
    m_active_nodes.clear(); m_orphan_nodes.clear();
    m_nodes.clear();        m_edges.clear();
  }
  
  /// \brief Add one nodes to the graph and returns the index of the node added.
  inline std::size_t add_node()
  {
    std::size_t offset = m_nodes.size();
    node* p_node = m_nodes.grow();
    // initialize the new node since m_nodes.grow() will not call the constructor of node
    p_node->m_out_edges = new edge_p_cont_type(); 
    return offset;
  }
  
  /// \brief Add fixed number (cnt) of nodes to the graph and returns the index of the first node added.
  ///
  /// If cnt is larger than one, this is faster than calling add_node() cnt times.
  /// For LOGISMOS graph with same number nodes per column, we only need to call this function once to add all nodes.
  /// If the graph has varying number of nodes per column, call this function for each column is better since it returns
  /// the index of the first node in the new column added.
  inline std::size_t add_nodes(std::size_t cnt)
  {
    if(cnt == 1)  return add_node();
    std::size_t offset=m_nodes.size();
    m_nodes.grow(cnt);
    std::size_t i(0);
    for(node* p_node = m_nodes.scan_start(offset); p_node; p_node = m_nodes.scan_next()){
      // initialize the new node since m_nodes.grow() will not call the constructor of node
      p_node->m_out_edges = new edge_p_cont_type(); 
      i++;
    }
    return offset;
  }
  
  /// \brief Get total number of nodes in the graph.
  inline std::size_t get_node_cnt(){  return m_nodes.size();  }
  
  /// \brief Add NEW terminal edges 'source->i' and 'i->sink' with given capacities.
  ///
  /// \param s_cap capacity for edge source->i.
  /// \param t_cap capacity for edge i->sink.
  /// \return true if success, false otherwise.
  /// \note If this function is called multiple times, only the first time will has real effect and returns true.
  /// \note There are no REAL terminal edges in the graph. They manifest as residual capacities of nodes.
  inline bool add_st_edge(std::size_t i, _Cap s_cap, _Cap t_cap)
  {
    assert(i<m_nodes.size());
    assert(s_cap>=0);
    assert(t_cap>=0);
    
    node* p_node = m_nodes.ptr_at(i);
    p_node->m_time = 0;
    if(p_node->is_active() == false){
      p_node->m_rcap = s_cap - t_cap;
      m_flow += (s_cap < t_cap) ? s_cap : t_cap;
      if(s_cap != t_cap){
        p_node->set_sink(p_node->m_rcap < 0);
        p_node->set_terminal();
        p_node->m_dist = 1;
        p_node->set_active(true);
      }
      else{
        p_node->m_par_edge = 0; // just make sure
      }
      return true;
    }
    return false;
  }
  
  /// \brief Add a NEW non-terminal edge from node i to node j.
  ///
  /// \param fwd_cap non-negative capacity from i to j.
  /// \param rev_cap non-negative capacity from j to i.
  /// \return the index of the new edge, i.e. number of edges before the new one is added.
  /// When used for adding intra-column edges which often have infinite capacity, 
  /// the user need to provide proper 'infinity' value for cap.
  /// \note This function ALWAYS add a new edge (i,j) to the graph even if edge (i,j) already exists.
  /// The user is responsible for using this function properly.
  inline std::size_t add_edge(std::size_t i, std::size_t j, _Cap fwd_cap, _Cap rev_cap = 0)
  {
    
    std::size_t old_size = m_edges.size();
    node* node_i = m_nodes.ptr_at(i);
    node* node_j = m_nodes.ptr_at(j);
    
    edge* fwd_edge = m_edges.grow();
    edge* rev_edge = m_edges.grow();
    fwd_edge->m_head = node_j;
    fwd_edge->m_rcap = fwd_cap;
    fwd_edge->m_sister = rev_edge;
    rev_edge->m_head = node_i;
    rev_edge->m_rcap = rev_cap;
    rev_edge->m_sister = fwd_edge;
        
    node_i->m_out_edges->push_back(fwd_edge);
    node_j->m_out_edges->push_back(rev_edge);
    
    return old_size;
  }
  
  /// \brief Get total number of non-terminal edges in the graph.
  inline std::size_t get_edge_cnt(){  return m_edges.size();  }
  
  /// \brief Get the number of non-terminal edges start from node i.
  inline std::size_t get_outgoing_edge_cnt(std::size_t i){  assert(i<m_nodes.size()); return m_nodes[i].m_out_edges->size();  }
  
  /// \brief Solve the maximum-flow/minimum s-t cut problem and returns the maximum flow value.
  ///
  /// \note Not used for search tree reuse.
  _Cap solve();
  
  /// \brief Determines if the given node is in the source set of the cut.
  ///
  /// \param  i index of the node
  /// \return true if the given node is the source set, false otherwise.
  inline bool in_source_set(std::size_t i)
  {
    assert(i<m_nodes.size());
    node* p_node = m_nodes.ptr_at(i);
    return (p_node->is_sink() == false) && p_node->has_parent();
  }
  
};  // end of class graph

} // end of namespace

#include "logismos_graph.cxx"

#endif
