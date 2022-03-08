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
  using Self = OSFSurface;
  using Superclass = Object;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  ITK_DISALLOW_COPY_AND_ASSIGN(OSFSurface);
  
  itkNewMacro(Self);
  itkTypeMacro(OSFSurface, Object);
  
  // utility type alias that helps defining cells and coordinates
  using MeshTraits = TSurfaceMeshTraits;
  
  // type aliases for identifiers
  using IdentifierType = unsigned long;
  using VertexIdentifier = IdentifierType;
  using ColumnPositionIdentifier = IdentifierType;
  using CellIdentifier = IdentifierType;

  // type aliases for a column of an OSF surface
  using  CoordRepType= typename MeshTraits::CoordRepType;
  static constexpr unsigned int PointDimension = MeshTraits::PointDimension;
  using CoordinateType = typename MeshTraits::PointType;
  using ColumnCoordinatesContainer = VectorContainer< ColumnPositionIdentifier, CoordinateType >;
  using ColumnCostType = TCostType;
  using ColumnCostsContainer = VectorContainer< ColumnPositionIdentifier, ColumnCostType >;
  
  // type aliases for a vertex of an OSF surface (not a coordinate, but a whole column)
  using VertexColumnCoordinatesContainer = VectorContainer< VertexIdentifier, typename ColumnCoordinatesContainer::Pointer >;
  using VertexColumnCostsContainer = VectorContainer< VertexIdentifier, typename ColumnCostsContainer::Pointer >;
  using VertexPositionIdentifierContainer = VectorContainer< VertexIdentifier, ColumnPositionIdentifier >;
  
  // type aliases for a cell of an OSF surface
  using  CellTraits= typename MeshTraits::CellTraits ;
  using CellPixelType = typename MeshTraits::CellPixelType;
  using CellType = CellInterface< CellPixelType, CellTraits >;
  using CellRawPointer = typename CellType::CellRawPointer;
  using CellAutoPointer = typename CellType::CellAutoPointer;
  using CellsContainer = typename MeshTraits::CellsContainer;
  
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
  using VertexIdentifierContainer = VectorContainer< unsigned int, VertexIdentifier >;
  const VertexIdentifierContainer* GetNeighbors(VertexIdentifier vertexId) const;
  void BuildNeighborLookupTable();

  // todo: do we need cell links like provided by the itk::Mesh?
  //using PointCellLinksContainer = typename MeshTraits::PointCellLinksContainer; // todo: do we need this?
  //using CellLinksContainer = typename MeshTraits::CellLinksContainer; // todo: do we need this?
  
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
  
  using VertexIdentifierListContainer = VectorContainer< VertexIdentifier, typename VertexIdentifierContainer::Pointer >;
  VertexIdentifierListContainer::Pointer m_VertexNeighborLookupTable;
  
private:
  void AddVertexNeighbors(VertexIdentifier vertex1, VertexIdentifier vertex2);
  
}; // end class OSFSurface

} // namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOSFSurface.txx"
#endif

#endif

