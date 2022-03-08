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

#ifndef _itkOSFSurface_h
#define _itkOSFSurface_h

#include <itkDataObject.h>
#include <itkObject.h>
#include <itkMesh.h>

namespace itk
{
/**\class OSFSurface
 * \brief
 * \date	12/9/2014
 * \author	Christian Bauer
 * ?
 * Template parameters for class OSFSurface:
 *
 * - TCostType = ?
 * - TSurfaceMeshTraits = ?
 */
 
template <typename TCostType, class TSurfaceMeshTraits=DefaultStaticMeshTraits<float, 3, 3> >
class ITK_EXPORT OSFSurface : public Object // todo: should probably change to DataObject
{
public:
  typedef OSFSurface Self;
  typedef Object Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  ITK_DISALLOW_COPY_AND_ASSIGN(OSFSurface);
  
  itkNewMacro(Self);
  itkTypeMacro(OSFSurface, Object);
  
  // utility typedef that helps defining cells and coordinates
  typedef TSurfaceMeshTraits MeshTraits;
  //typedef DefaultStaticMeshTraits<char, 3> MeshTraits;
  
  // typedefs for identifiers
  typedef unsigned long IdentifierType;
  typedef IdentifierType VertexIdentifier;
  typedef IdentifierType ColumnPositionIdentifier;
  typedef IdentifierType CellIdentifier;

  // typedefs for a column of an OSF surface
  typedef typename MeshTraits::CoordRepType CoordRepType;
  itkStaticConstMacro(PointDimension, unsigned int, MeshTraits::PointDimension);
  typedef typename MeshTraits::PointType CoordinateType;
  typedef VectorContainer< ColumnPositionIdentifier, CoordinateType > ColumnCoordinatesContainer;
  typedef TCostType ColumnCostType;
  typedef VectorContainer< ColumnPositionIdentifier, ColumnCostType > ColumnCostsContainer;
  
  // typedefs for a vertex of an OSF surface (not a coordinate, but a whole column)
  typedef VectorContainer< VertexIdentifier, typename ColumnCoordinatesContainer::Pointer > VertexColumnCoordinatesContainer;
  typedef VectorContainer< VertexIdentifier, typename ColumnCostsContainer::Pointer > VertexColumnCostsContainer;
  typedef VectorContainer< VertexIdentifier, ColumnPositionIdentifier > VertexPositionIdentifierContainer;
  
  // typedefs for a cell of an OSF surface
  typedef typename MeshTraits::CellTraits CellTraits;
  typedef typename MeshTraits::CellPixelType CellPixelType;
  typedef CellInterface< CellPixelType, CellTraits > CellType;
  typedef typename CellType::CellRawPointer CellRawPointer;
  typedef typename CellType::CellAutoPointer CellAutoPointer;
  typedef typename MeshTraits::CellsContainer CellsContainer;
  
  VertexIdentifier GetNumberOfVertices( ) const;
  ColumnPositionIdentifier GetNumberOfColumns(VertexIdentifier vertexId) const;
  
  // access to column coordinates of a vertex
  ColumnCoordinatesContainer* GetColumnCoordinates(VertexIdentifier vertexId);
  const ColumnCoordinatesContainer* GetColumnCoordinates(VertexIdentifier vertexId) const;
  void SetColumnCoordinates(VertexIdentifier vertexId, ColumnCoordinatesContainer* columnCoordinates);
  
  // access to column costs of a vertex
  ColumnCostsContainer* GetColumnCosts(VertexIdentifier vertexId);
  const ColumnCostsContainer* GetColumnCosts(VertexIdentifier vertexId) const;
  void SetColumnCosts(VertexIdentifier vertexId, ColumnCostsContainer* columnCosts);
  
  // access to initial vertex position
  const CoordinateType& GetInitialVertexPosition(VertexIdentifier vertexId) const;
  ColumnPositionIdentifier GetInitialVertexPositionIdentifier(VertexIdentifier vertexId) const;
  void SetInitialVertexPositionIdentifier(VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId);
  
  // access to current vertex position
  const CoordinateType& GetCurrentVertexPosition(VertexIdentifier vertexId) const;
  ColumnPositionIdentifier GetCurrentVertexPositionIdentifier(VertexIdentifier vertexId) const;
  void SetCurrentVertexPositionIdentifier(VertexIdentifier vertexId, ColumnPositionIdentifier columnPositionId);
  
  // access to cells
  CellsContainer* GetCells();
  const CellsContainer* GetCells() const;
  void SetCells(CellsContainer* cells);
  
  // access to individual cells
  CellIdentifier GetNumberOfCells() const;
  bool GetCell(CellIdentifier cellId, CellAutoPointer& cellPointer) const;
  void SetCell(CellIdentifier cellId, CellAutoPointer& cellPointer);

  // allow lookup of neighboring of columns on the surface
  typedef VectorContainer< unsigned int, VertexIdentifier > VertexIdentifierContainer;
  const VertexIdentifierContainer* GetNeighbors(VertexIdentifier vertexId) const;
  void BuildNeighborLookupTable();

  // todo: do we need cell links like provided by the itk::Mesh?
  //typedef typename MeshTraits::PointCellLinksContainer PointCellLinksContainer; // todo: do we need this?
  //typedef typename MeshTraits::CellLinksContainer CellLinksContainer; // todo: do we need this?
  
protected:
  /** Constructor for use by New() method. */
  OSFSurface() = default;
  ~OSFSurface() override = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;
  
  typename VertexColumnCoordinatesContainer::Pointer m_VertexColumnCoordinatesContainer{ VertexColumnCoordinatesContainer::New() };
  typename VertexColumnCostsContainer::Pointer m_VertexColumnCostsContainer{ VertexColumnCostsContainer::New() };
  VertexPositionIdentifierContainer::Pointer m_VertexInitialPositionIdentifierContainer{ VertexPositionIdentifierContainer::New() };
  VertexPositionIdentifierContainer::Pointer m_VertexCurrentPositionIdentifierContainer{ VertexPositionIdentifierContainer::New() };
  typename CellsContainer::Pointer m_CellsContainer{ CellsContainer::New() };
  
  void ReleaseCellsMemory();
  
  typedef VectorContainer< VertexIdentifier, typename VertexIdentifierContainer::Pointer > VertexIdentifierListContainer;
  VertexIdentifierListContainer::Pointer m_VertexNeighborLookupTable;
  
private:
  void AddVertexNeighbors(VertexIdentifier vertex1, VertexIdentifier vertex2);
  
}; // end class OSFSurface

} // namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFSurface.txx"
#endif

#endif

