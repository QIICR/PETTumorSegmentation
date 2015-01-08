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

namespace LOGISMOS{

////////////////////////////////////////////////////////
template <typename _Cap, std::size_t _DataChunkSize, std::size_t _PtrChunkSize>
_Cap graph<_Cap, _DataChunkSize, _PtrChunkSize>::solve()
{
  // make sure we start from correct initial condition
  m_orphan_nodes.clear();
  
  node* p_node;
  edge* p_edge;
  
  p_node=m_nodes.scan_first();
  while(p_node){
    if(p_node->has_parent() == false || p_node->is_active() == false)
      p_edge = 0;
    else{
      p_edge = grow_active_node(p_node);
      m_clock++;
    }

    if(p_edge){ // will process same active node in next iteration
      augment_path(p_edge);
      while(m_orphan_nodes.empty() == false){
        adopt_orphan(m_orphan_nodes.front());
        m_orphan_nodes.pop_front();
      }
    }
    else{ // ready to process next node
      p_node->set_active(false);
      p_node = m_nodes.scan_next();
    }
  }
  
  while(m_active_nodes.empty() == false){
    p_node = m_active_nodes.front();  // p_node->is_active() is always true here
    if(p_node->has_parent() == false) // a node in the queue may lose parent during adopt_orphan()
      p_edge = 0;
    else{
      p_edge = grow_active_node(p_node);
      m_clock++;
    } 
    
    if(p_edge){ // will process same active node in next iteration
      augment_path(p_edge);
      while(m_orphan_nodes.empty() == false){
        adopt_orphan(m_orphan_nodes.front());
        m_orphan_nodes.pop_front();
      }
    }
    else{ // ready to process next node in active queue
      p_node->set_active(false);
      m_active_nodes.pop_front();
    }
  }
  return m_flow;
} 

////////////////////////////////////////////////////////
template <typename _Cap, std::size_t _DataChunkSize, std::size_t _PtrChunkSize>
typename graph<_Cap, _DataChunkSize, _PtrChunkSize>::edge* graph<_Cap, _DataChunkSize, _PtrChunkSize>::grow_active_node(node* node_i)
{
  bool i_is_sink = node_i->is_sink();
  
  for(edge** edge_dptr  = node_i->m_out_edges->scan_first(); edge_dptr; edge_dptr = node_i->m_out_edges->scan_next()){
    edge* p_edge = *edge_dptr;
    _Cap  cap = (i_is_sink) ? p_edge->m_sister->m_rcap : p_edge->m_rcap;
    if(cap == 0)  continue; // only check edge with residual capacity
    
    node* node_j = p_edge->m_head;
    if(node_j->has_parent() == false){
      node_j->set_sink(i_is_sink);  // assign j to the same tree
      node_j->m_par_edge = p_edge->m_sister;
      node_j->m_time = node_i->m_time;
      node_j->m_dist = node_i->m_dist + 1;
      activate(node_j);
    }
    else if(node_j->is_sink() != i_is_sink){
      // return the edge from a source node to a sink node
      if(i_is_sink) return p_edge->m_sister;  
      else  return p_edge;  
    }
    else if(node_j->m_time <= node_i->m_time && node_j->m_dist > node_i->m_dist){
      // trying to make the distance from j to the terminal node shorter
      node_j->m_par_edge = p_edge->m_sister;
      node_j->m_time = node_i->m_time;
      node_j->m_dist = node_i->m_dist + 1;
    }
  }
  return 0; // found NO edge connecting source and sink trees
}

////////////////////////////////////////////////////////
template <typename _Cap, std::size_t _DataChunkSize, std::size_t _PtrChunkSize>
void graph<_Cap, _DataChunkSize, _PtrChunkSize>::augment_path(edge* mid_edge)
{
  edge* p_edge;
  _Cap cap;
  _Cap eps = std::numeric_limits<_Cap>::epsilon();
  _Cap bottleneck = mid_edge->m_rcap; // initial bottleneck

  // find bottleneck residual capacity of source tree
  node* p_node = mid_edge->m_sister->m_head;
  while(p_node->is_terminal() == false){
    p_edge = p_node->m_par_edge;
    cap = p_edge->m_sister->m_rcap;
    if(bottleneck > cap)  bottleneck = cap;
    p_node = p_edge->m_head;
  }
  if(bottleneck > p_node->m_rcap)   bottleneck = p_node->m_rcap;
  
  // find bottleneck residual capacity of sink tree
  p_node = mid_edge->m_head;
  while(p_node->is_terminal() == false){
    p_edge = p_node->m_par_edge;
    cap = p_edge->m_rcap;
    if(bottleneck > cap)    bottleneck = cap;
    p_node = p_edge->m_head;
  }
  if(bottleneck > -(p_node->m_rcap))  bottleneck = -(p_node->m_rcap);
  
  
  // augment the middle edge
  mid_edge->m_rcap -= bottleneck;
  mid_edge->m_sister->m_rcap += bottleneck;
  
  // augment the source tree
  p_node = mid_edge->m_sister->m_head;
  while(p_node->is_terminal() == false){
    p_edge = p_node->m_par_edge;
    p_edge->m_rcap += bottleneck;
    p_edge->m_sister->m_rcap -= bottleneck;
    if(p_edge->m_sister->m_rcap <= eps){
      p_edge->m_sister->m_rcap = 0;
      mark_orphan(p_node);
    } 
    p_node = p_edge->m_head;
  }
  p_node->m_rcap -= bottleneck;
  if(p_node->m_rcap <= eps){
    p_node->m_rcap = 0;
    mark_orphan(p_node);
  } 
  
  // augment the sink tree
  p_node = mid_edge->m_head;
  while(p_node->is_terminal() == false){
    p_edge = p_node->m_par_edge;
    p_edge->m_rcap -= bottleneck;
    p_edge->m_sister->m_rcap += bottleneck;
    if(p_edge->m_rcap <= eps){
      p_edge->m_rcap = 0;
      mark_orphan(p_node);
    } 
    p_node = p_edge->m_head;
  }
  p_node->m_rcap += bottleneck;
  if(-p_node->m_rcap <= eps){
    p_node->m_rcap = 0;
    mark_orphan(p_node);
  } 
  
  m_flow += bottleneck;
}

////////////////////////////////////////////////////////
template <typename _Cap, std::size_t _DataChunkSize, std::size_t _PtrChunkSize>
void graph<_Cap, _DataChunkSize, _PtrChunkSize>::adopt_orphan(node* node_i)
{
  bool i_is_sink = node_i->is_sink(); // which tree the orphan node belong, sink (true) or source (false)
  node* node_j;
  edge* p_edge;
  
  // try to find a new parent for node i
  unsigned int dist; 
  unsigned int d_max = std::numeric_limits<unsigned int>::max();
  unsigned int min_dist = std::numeric_limits<unsigned int>::max();
  edge* min_p_edge = 0;   // starting from this edge, can backtrack to terminal w/ minimal distance (# of hops)

  for(edge** edge_dptr  = node_i->m_out_edges->scan_first(); edge_dptr; edge_dptr = node_i->m_out_edges->scan_next() ){
    p_edge = *edge_dptr;
    node_j = p_edge->m_head;
    _Cap cap = (i_is_sink) ? p_edge->m_rcap : p_edge->m_sister->m_rcap;
    // candidate node j must satisfy: 
    // 1) the edge between i and j is not saturated, 
    // 2) it belongs to the same tree as node i, 
    // 3) it has a parent (not an orphan)
    if(cap == 0 || node_j->is_sink() != i_is_sink || node_j->has_parent() == false) continue;
    
    // node j can become parent for node i only if its originates from the same type of terminal node as node i
    // i.e. we can backtrack to terminal node along parent edges without meeting a orphan node.
    dist = 0;   // distance to terminal node
    while(true){
      if(node_j->m_time == m_clock){  // node j was update at the same m_clock --> its origin is already validated
        dist += node_j->m_dist; break;  
      }
      dist++;
      if(node_j->is_terminal()){
        node_j->m_time = m_clock; node_j->m_dist = 1; break;
      }
      if(node_j->is_orphan()){
        dist = d_max; break;
      }
      node_j = node_j->m_par_edge->m_head;
    }
    
    if(dist < d_max){ // node j's origin is valid
      if(dist < min_dist){      // we prefer a path such that node j that is closest to the terminal
        min_p_edge = p_edge;  min_dist = dist;
      }
      // update time stamps and distance along the path to terminal
      for(node_j = p_edge->m_head; node_j->m_time != m_clock; node_j = node_j->m_par_edge->m_head){
        node_j->m_time = m_clock;   node_j->m_dist = dist;  dist--;
      }
    }
  }
    
  node_i->m_par_edge = min_p_edge;  // update i's parent -- zero: not found
  if(min_p_edge != 0){
    node_i->m_time = m_clock;
    node_i->m_dist = min_dist+1;
  }
  else{
    // 1) activate i's neighbors that may claim i as child (positive residual capacity on associated edge)
    // 2) node i's children then become orphans
    for(edge** edge_dptr  = node_i->m_out_edges->scan_first(); edge_dptr; edge_dptr = node_i->m_out_edges->scan_next() ){
      p_edge = *edge_dptr;
      node_j = p_edge->m_head;
      if(node_j->is_sink() == i_is_sink && node_j->has_parent()){
        _Cap cap = (i_is_sink) ? p_edge->m_rcap : p_edge->m_sister->m_rcap;
        if(cap!=0){
          activate(node_j);
        }
        if(node_j->is_terminal() == false && node_j->is_orphan() == false && node_j->m_par_edge->m_head == node_i){
          mark_orphan(node_j);
        }
      }
    }
  }
}

} // end of namespace
