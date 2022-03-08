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

#ifndef _itkOSFSurface_txx
#define _itkOSFSurface_txx

#include "itkOSFSurface.h"

namespace itk
{

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::VertexIdentifier
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetNumberOfVertices() const
{
  return  m_VertexColumnCoordinatesContainer->Size();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnCoordinatesContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetColumnCoordinates(VertexIdentifier vertexId)
{
  if ( !m_VertexColumnCoordinatesContainer->IndexExists(vertexId) || !m_VertexColumnCoordinatesContainer->GetElement(vertexId) )
    {
      m_VertexColumnCoordinatesContainer->InsertElement( vertexId, ColumnCoordinatesContainer::New() );
    }
  return m_VertexColumnCoordinatesContainer->GetElement( vertexId );
}
  
//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnCoordinatesContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetColumnCoordinates(VertexIdentifier vertexId) const
{
  return m_VertexColumnCoordinatesContainer->GetElement( vertexId ).GetPointer();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetColumnCoordinates(VertexIdentifier vertexId, ColumnCoordinatesContainer* columnCoordinates)
{
  if ( !m_VertexColumnCoordinatesContainer->IndexExists(vertexId) )
  {
    m_VertexColumnCoordinatesContainer->InsertElement( vertexId, columnCoordinates );
    this->Modified();
  }
  else if ( m_VertexColumnCoordinatesContainer->GetElement(vertexId)!=columnCoordinates )
  {
    m_VertexColumnCoordinatesContainer->SetElement( vertexId, columnCoordinates );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnPositionIdentifier
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetNumberOfColumns(VertexIdentifier vertexId) const
{
  typename ColumnCoordinatesContainer::Pointer columnCoordinatesContainer;
  if ( m_VertexColumnCoordinatesContainer->GetElementIfIndexExists(vertexId,&columnCoordinatesContainer) )
    if ( columnCoordinatesContainer )
      return columnCoordinatesContainer->Size();
  return 0;
}


//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnCostsContainer* 
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetColumnCosts(VertexIdentifier vertexId)
{
  if ( !m_VertexColumnCostsContainer->IndexExists(vertexId) || !m_VertexColumnCostsContainer->GetElement(vertexId) )
    {
      m_VertexColumnCostsContainer->InsertElement( vertexId, ColumnCostsContainer::New() ); // create container if it does not exists yet
    }
  return m_VertexColumnCostsContainer->GetElement( vertexId );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnCostsContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetColumnCosts(VertexIdentifier vertexId) const
{
  return m_VertexColumnCostsContainer->GetElement( vertexId ).GetPointer();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetColumnCosts(VertexIdentifier vertexId, ColumnCostsContainer* columnCosts)
{
  if ( !m_VertexColumnCostsContainer->IndexExists(vertexId) )
  {
    m_VertexColumnCostsContainer->InsertElement( vertexId, columnCosts ); // create container if it does not exists yet
    this->Modified();
  }
  else if (m_VertexColumnCostsContainer->GetElement(vertexId)!=columnCosts )
  {
    m_VertexColumnCostsContainer->SetElement( vertexId, columnCosts );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::CoordinateType&
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetInitialVertexPosition(VertexIdentifier vertexId) const
{
  const ColumnPositionIdentifier columnId = this->GetInitialVertexPositionIdentifier( vertexId );
  // todo: checks if it exists or not possible, but what to return in this case?
  return m_VertexColumnCoordinatesContainer->GetElement(vertexId)->ElementAt(columnId);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnPositionIdentifier
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetInitialVertexPositionIdentifier(VertexIdentifier vertexId) const
{
  if ( !m_VertexInitialPositionIdentifierContainer->IndexExists(vertexId) )
    return 0;
  return m_VertexInitialPositionIdentifierContainer->GetElement(vertexId);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetInitialVertexPositionIdentifier(VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId)
{
  if ( !m_VertexInitialPositionIdentifierContainer->IndexExists(vertexId) )
  {
    m_VertexInitialPositionIdentifierContainer->InsertElement( vertexId, columnPositionId );
    this->Modified();
  }
  else if (m_VertexInitialPositionIdentifierContainer->GetElement(vertexId)!=columnPositionId)
  {
    m_VertexInitialPositionIdentifierContainer->SetElement( vertexId, columnPositionId );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::CoordinateType&
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetCurrentVertexPosition(VertexIdentifier vertexId) const
{
  const ColumnPositionIdentifier columnId = this->GetCurrentVertexPositionIdentifier( vertexId );
  // todo: checks if it exists or not possible, but what to return in this case?
  return m_VertexColumnCoordinatesContainer->GetElement(vertexId)->ElementAt(columnId);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::ColumnPositionIdentifier
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetCurrentVertexPositionIdentifier(VertexIdentifier vertexId) const
{
  if ( !m_VertexCurrentPositionIdentifierContainer->IndexExists(vertexId) )
    return 0;
  return m_VertexCurrentPositionIdentifierContainer->GetElement(vertexId);
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetCurrentVertexPositionIdentifier(VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId)
{
  if ( !m_VertexCurrentPositionIdentifierContainer->IndexExists(vertexId) )
  {
    m_VertexCurrentPositionIdentifierContainer->InsertElement( vertexId, columnPositionId );
    this->Modified();
  }
  else if (m_VertexCurrentPositionIdentifierContainer->GetElement(vertexId)!=columnPositionId)
  {
    m_VertexCurrentPositionIdentifierContainer->SetElement( vertexId, columnPositionId );
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::CellsContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetCells()
{
  return m_CellsContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::CellsContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetCells() const
{
  return m_CellsContainer;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetCells(CellsContainer* cells)
{
  if(m_CellsContainer != cells)
  {
    this->ReleaseCellsMemory();
    m_CellsContainer = cells;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::ReleaseCellsMemory()
{
  // todo: might require rework!!!
  // copied from itk::Mesh assuming that only the default cell allocation method of "CellsAllocatedDynamicallyCellByCell" is used
  if ( m_CellsContainer && m_CellsContainer->GetReferenceCount()==1 )
  {
    // It is assumed that every cell was allocated independently.
    // A Cell iterator is created for going through the cells
    // deleting one by one.
    typename CellsContainer::Iterator cell  = m_CellsContainer->Begin();
    typename CellsContainer::Iterator end   = m_CellsContainer->End();
    while( cell != end )
    {
      const CellType * cellToBeDeleted = cell->Value();
      delete cellToBeDeleted;
      ++cell;
    }
    m_CellsContainer->Initialize(); 
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
typename OSFSurface<TCostType, TSurfaceMeshTraits>::CellIdentifier
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetNumberOfCells() const
{
  if ( ! m_CellsContainer )
    return 0;
  else
    return m_CellsContainer->Size();
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
bool
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetCell(CellIdentifier cellId, CellAutoPointer& cellPointer) const
{
  // If the cells container doesn't exist, then the cell doesn't exist.
  if( m_CellsContainer.IsNull() )
  {
    cellPointer.Reset();
    return false;
  }

  // Ask the container if the cell identifier exists.
  CellType * cellptr = 0;
  const bool found = m_CellsContainer->GetElementIfIndexExists(cellId, &cellptr);
  if( found )
    cellPointer.TakeNoOwnership( cellptr );
  else
    cellPointer.Reset();

  return found;
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::SetCell(CellIdentifier cellId, CellAutoPointer& cellPointer)
{
  // Make sure a cells container exists.
  if( !m_CellsContainer )
    this->SetCells(CellsContainer::New());

  // Insert the cell into the container with the given identifier.
  m_CellsContainer->InsertElement(cellId, cellPointer.ReleaseOwnership() );
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  // todo: implement
}
//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
const typename OSFSurface<TCostType, TSurfaceMeshTraits>::VertexIdentifierContainer*
OSFSurface<TCostType, TSurfaceMeshTraits>
::GetNeighbors(VertexIdentifier vertexId) const
{
  if (!m_VertexNeighborLookupTable || !m_VertexNeighborLookupTable->IndexExists(vertexId) )
    return 0;
  return m_VertexNeighborLookupTable->ElementAt(vertexId);  
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::BuildNeighborLookupTable()
{
  m_VertexNeighborLookupTable = VertexIdentifierListContainer::New();
  VertexIdentifier numVertices = this->GetNumberOfVertices();
  m_VertexNeighborLookupTable->CreateIndex( numVertices );
  for (VertexIdentifier vertexId=0; vertexId<numVertices; vertexId++)
    m_VertexNeighborLookupTable->SetElement( vertexId, VertexIdentifierContainer::New() );
  
  for (typename CellsContainer::ConstIterator cellItr=m_CellsContainer->Begin(); cellItr!=m_CellsContainer->End(); cellItr++)
  {
    const CellType* cellPointer = cellItr.Value();
    unsigned int numberOfPoints = cellPointer->GetNumberOfPoints();
    if (numberOfPoints>1)
    {
      std::vector<VertexIdentifier> vertexIds(numberOfPoints);
      std::vector<VertexIdentifier>::iterator vertexIdsItr = vertexIds.begin();
      typename CellType::PointIdConstIterator pointIdIterator = cellPointer->PointIdsBegin();
      typename CellType::PointIdConstIterator pointIdEnd = cellPointer->PointIdsEnd();
    
      while( pointIdIterator != pointIdEnd )
      {
        *vertexIdsItr = *pointIdIterator;
        //std::cout << *pointIdIterator << std::endl;
        ++pointIdIterator; ++vertexIdsItr;
      }
      
      this->AddVertexNeighbors(vertexIds[0], vertexIds[numberOfPoints-1]);
      for (unsigned int i=0; i<numberOfPoints-1; i++)
        this->AddVertexNeighbors(vertexIds[i], vertexIds[i+1]);
      }
    //break;
  }
}

//----------------------------------------------------------------------------
template <typename TCostType, typename TSurfaceMeshTraits >
void
OSFSurface<TCostType, TSurfaceMeshTraits>
::AddVertexNeighbors(VertexIdentifier vertex1, VertexIdentifier vertex2)
{
  // todo: implement
  //std::cout << "adding neighborhood: " << vertex1 << " - " << vertex2 << std::endl; return; // for testing
  
  typename VertexIdentifierContainer::Pointer vertex1Neighbors = m_VertexNeighborLookupTable->ElementAt( vertex1 );
  bool newNeighbor = true;
  for (typename VertexIdentifierContainer::ConstIterator itr=vertex1Neighbors->Begin(); itr!=vertex1Neighbors->End(); itr++)
  {
    if (itr.Value()==vertex2)
     {newNeighbor=false; break;}
  }
  if (newNeighbor)
    vertex1Neighbors->push_back(vertex2);
  
  typename VertexIdentifierContainer::Pointer vertex2Neighbors = m_VertexNeighborLookupTable->ElementAt( vertex2 );
  newNeighbor = true;
  for (typename VertexIdentifierContainer::ConstIterator itr=vertex2Neighbors->Begin(); itr!=vertex2Neighbors->End(); itr++)
  {
    if (itr.Value()==vertex1)
     {newNeighbor=false; break;}
  }
  if (newNeighbor)
    vertex2Neighbors->push_back(vertex1);
}

} // namespace

#endif

