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

#ifndef _LOGISMOS_chunk_list_hxx_
#define _LOGISMOS_chunk_list_hxx_

#include <cstddef>  // for std::size_t
#include <cstring>  // for std::memset
#include <vector>
#include <iostream> // for testing only

namespace LOGISMOS{

/// \brief A continuous memory chunk used for data storage.
///
/// It emulates a std::vector<_T> with fixed capacity of _N that can never grow.
/// \note Only a small subset of member functions of std::vector is provided
/// because we assume this class is only directly used by chunk_list class.
template <typename _T, std::size_t _N>
class chunk
{
private:
  _T  m_data[_N]; ///< data buffer (container) with fixed capacity
  _T* m_end;      ///< pointer to the location after the last valid data elements
  
public: 
  /// \brief Constructor, data buffer is initially filled with zeros.
  ///
  /// \note Constructor of data type _T is NOT called.
  chunk()
  {
    std::memset(m_data, 0, sizeof(m_data));
    m_end = &(m_data[0]);
  }
	
  // member functions with same names and same or very similar meanings as std::vector
  inline _T* begin(){ return &(m_data[0]);  }
  inline _T* end()  { return m_end;   }
  inline std::size_t size() const{  return m_end - &(m_data[0]);  }
  inline bool empty() const{  return m_end == &(m_data[0]); }
  inline _T& operator[](std::size_t i){ return m_data[i]; }
  inline const _T& operator[] (std::size_t i) const{  return m_data[i]; }
  
  /// \brief Grow the chunk by cnt elements at the end.
  ///
  /// \return Pointer to the first new element.
  /// \note Boundary check is NOT performed.
  inline _T* grow(std::size_t cnt=1)
  { 
    _T* ptr = m_end;  // the old end is the first new element
    m_end += cnt;
    return ptr;
  }
};  // end of class chunk

/// \brief A series of chunks
///
/// This class is specially designed for the use case in which 
/// 1) a LARGE number of data elements need to be stored;
/// 2) the number of elements may not be known at construction; 
/// 3) the storage can dynamically grow with minimal penalty for performance.
/// Comparing with std::vector
/// 1) grow the chunk_list never requires large blocks of memory to be reallocated and copied;
/// 2) the amount of new memory allocated when growing is fixed instead of always doubling as in std::vector
/// 3) using scan_first() and scan_next() to sequentially access all elements is efficient (should be similar to std::vector);
/// 4) random access using ptr_at() or operator [] is easy and should be good enough
/// but may not be very efficient for sequential access of all elements.
template <typename _T, std::size_t _N>
class chunk_list
{
  typedef chunk<_T, _N>                 chunk_type;
  typedef std::vector<chunk_type*>      list_type;
  typedef typename list_type::iterator  list_iter_type;

private:
  list_type       m_list; ///< the data (chunk) container
  std::size_t     m_size; ///< actual number of elements of type _T stored, NOT the size of m_list
  
  _T*             m_data_ptr;   ///< pointer to the current element of type _T being accessed
  list_iter_type  m_chunk_iter; ///< iterator of the current chunk being accessed

public:
  /// \brief Constructor
  chunk_list() : m_size(0), m_data_ptr(0){  }
  
  /// \brief Destructor
  ~chunk_list(){  clear();  }
  
  /// \brief Clears all elements in the list and memory allocated.
  void clear()
  {
    m_size = 0; m_data_ptr = 0;
    for(list_iter_type iter = m_list.begin(); iter != m_list.end(); ++iter){  delete *iter; }
    m_list.clear();
    list_type().swap(m_list);   // actually frees memory allocated if list_type is std::vector
  }
  
  /// \brief Returns the total number of elements of type _T in the list.
  inline std::size_t size() const{  return m_size;  }
  
  /// \brief Returns number of chunks used by the list
  inline std::size_t chunks() const{  return m_list.size(); }
  
  /// \brief Returns pointer to the beginning of the list
  inline _T* begin(){ return m_list.front()->begin(); }
  
  /// \brief Returns pointer to the end of the list, i.e. pointer to location right after the last element.
  inline _T* end(){ return m_list.back()->end();  }
  
  /// \brief Returns pointer to the ith element (no boundary check).
  inline _T* ptr_at(std::size_t i)
  {
    std::size_t c(0); // which chunk
    std::size_t d(i); // index within chunk
    while(d >= _N){ c++;  d -= _N;  }
    return m_list[c]->begin()+d;
  }
  
  /// \brief Return reference to the ith element (no boundary check).
  inline _T& operator[](std::size_t i)
  {
    std::size_t c(0); // which chunk
    std::size_t d(i); // index within chunk
    while(d >= _N){ c++;  d -= _N;  }
    return (*m_list[c])[d];
  }
  
  /// \brief Returns const reference to the ith element (no boundary check).
  inline const _T& operator[](std::size_t i) const
  {
    std::size_t c(0); // which chunk
    std::size_t d(i); // index within chunk
    while(d >= _N){ c++;  d -= _N;  }
    return (*m_list[c])[d];
  }
  
  /// \brief Grow the chunk_list by one elements at the end (allocates a new chunk if needed) and returns pointer to the new element.
  inline _T* grow()
  {
    if(m_list.empty() || m_list.back()->size() == _N){  // list is empty or the last chunk is full
      m_list.push_back(new chunk_type()); // create a new chunk
    } 
    m_size++;
    return m_list.back()->grow();
  }
  
  /// brief Add a new element with given value specified in val.
  inline void push_back(const _T &val){ *grow() = val;  }

  /// \brief Grow the chunk_list by cnt elements, i.e. increase the total number of elements can be stored by cnt.
  ///
  /// \return index of the first element added, i.e. the total number of elements before the new ones are added.
  /// \note If cnt > _N, multiple chunks will be added, thus providing a quick way to allocate memory.
  std::size_t grow(std::size_t cnt)
  {
    std::size_t old_size = m_size;
    
    if(cnt == 1){ grow(); return old_size;  }
    if(m_list.empty() || m_list.back()->size() == _N){  // list is empty or the last chunk is full
      m_list.push_back(new chunk_type()); // create a new chunk
    }
    
    std::size_t grow_slots = cnt;
    std::size_t free_slots = _N - m_list.back()->size();
    if(grow_slots <= free_slots){ // have enough free slots in the last chunk, grow and return
      m_list.back()->grow(grow_slots);  m_size += grow_slots; return old_size;
    }
    else{ // not enough space in the last chunk, fill it first
      m_list.back()->grow(free_slots);  m_size += free_slots;
      grow_slots -= free_slots;
    }
    
    std::size_t chunk_cnt = grow_slots / _N;  // number of chunk to grow
    std::size_t extra_cnt = grow_slots % _N;  // number of additional elements to grow that is not a full chunk
    for(std::size_t i=0;i<chunk_cnt;++i){
      m_list.push_back(new chunk_type()); 
      m_list.back()->grow(_N);  m_size += _N;
    }
    if(extra_cnt > 0){
      m_list.push_back(new chunk_type()); 
      m_list.back()->grow(extra_cnt); m_size += extra_cnt;
    }
    return old_size;
  }
  
  /// \brief Returns pointer to the first element to access, 0 if chunk_list is empty
  ///
  /// Use this to start a loop for all elements.
  inline _T* scan_first()
  {
    if(m_list.empty())  return 0;
    m_chunk_iter = m_list.begin();
    m_data_ptr = (*m_chunk_iter)->begin();
    return m_data_ptr;
  }
  
  /// \brief Returns pointer to the ith element, 0 if chunk_list is empty
  ///
  /// Use this to start a loop starting from the ith element.
  inline _T* scan_start(std::size_t i)
  {
    if(m_list.empty())  return 0;
    std::size_t c(0); // which chunk
    std::size_t d(i); // index within chunk
    while(d >= _N){ c++;  d -= _N;  }
    m_chunk_iter = m_list.begin()+c;
    m_data_ptr = (*m_chunk_iter)->begin()+d;
    return m_data_ptr;
  }
  
  /// \brief Returns pointer to the next element to access.
  ///
  /// Use this to continue looping through all elements, 
  /// will return 0 when reaching the end of the list.
  /// \note Must call scan_first() or scan_start(i) before calling this one, will return 0 otherwise.
  inline _T* scan_next()
  {
    if(!m_data_ptr) return 0;
    m_data_ptr++;
    if(m_data_ptr == (*m_chunk_iter)->end()){ // already pointing to the last element of a chunk
      m_chunk_iter++; // move to the next chunk
      if(m_chunk_iter == m_list.end())  // reached the last chunk
        m_data_ptr = 0;
      else
        m_data_ptr = (*m_chunk_iter)->begin();
    }
    return m_data_ptr;
  }
  
  /// \brief Print elements stored (one chunk a line), for testing purpose only
  void print()
  {
    std::cout<<this->chunks()<<" chunks, "<<this->size()<<" elements"<<std::endl;
    for(list_iter_type iter = m_list.begin(); iter != m_list.end(); ++iter)
      (*iter)->print();
  }
};  // end of class chunk_list

} // end of namespace
#endif
