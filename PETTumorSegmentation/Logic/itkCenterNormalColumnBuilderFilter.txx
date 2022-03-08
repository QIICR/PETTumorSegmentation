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

#ifndef _itkCenterNormalColumnBuilderFilter_txx
#define _itkCenterNormalColumnBuilderFilter_txx

#include "itkCenterNormalColumnBuilderFilter.h"


/*
Differences from itkSurfaceNormalColumnBuilderFilter:
1. Normal for point to center rather surface outwards
2. No steps backward.
3. Define either step length of number of steps.  One will define the other.
4. Must define the center of the region.
*/

namespace itk
{

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::GenerateData()
{  
  if( this->GetInput()->GetNumberOfSurfaces()!=1 )
  {
    itkWarningMacro("CenterNormalColumnBuilderFilter currently only supports one surface!");
  }
  
  //Copy the basic surface information.
  this->CopyInputOSFGraphToOutputOSFGraphSurfaces();

  //Create cells based on the vertices.
  BuildVertexToCellLookupTable();
  
  OutputOSFGraphPointer output = this->GetOutput();
  typename OSFSurface::Pointer surface = output->GetSurface();

  //Build each column, one per vertex.
  for (VertexIdentifier vertexId=0; vertexId<surface->GetNumberOfVertices(); vertexId++)
  {  this->BuildColumn(vertexId);}

}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::BuildColumn(VertexIdentifier vertexId)
{
  typename OSFSurface::ConstPointer inputSurface =  this->GetInput()->GetSurface();
  typename OSFSurface::Pointer outputSurface =  this->GetOutput()->GetSurface();
  
  using Coordinate = typename OSFSurface::CoordinateType;
  using ColumnCoordinatesContainer = typename OSFSurface::ColumnCoordinatesContainer;
  using ColumnCostsContainer = typename OSFSurface::ColumnCostsContainer;
  
  Coordinate initialPosition;
  initialPosition[0] = m_CenterPoint[0];
  initialPosition[1] = m_CenterPoint[1];
  initialPosition[2] = m_CenterPoint[2];

  //Determine proper direction vector from center to vertex with magnitude 1.
  DirectionVector CenterNormalDirection = GetNormal(vertexId);
  typename ColumnCoordinatesContainer::Pointer columnPositions = ColumnCoordinatesContainer::New();
  columnPositions->CreateIndex( m_NumberOfSteps-1 );

  
  // build column coordinates
  // Given a vector of magnitude 1 away from the center and the center as an initial position,
  // multiply the vector by the number of steps (from 1 to the maximum number of nodes) and the
  // length of each step.
  
  // center outward
  for (unsigned int step=0; step<m_NumberOfSteps; step++)
  {
    Coordinate currentPosition = initialPosition + m_StepLength*(step+1)*CenterNormalDirection;
    columnPositions->SetElement( step, currentPosition );
  }

  //Apply these coordinates to the graph
  outputSurface->SetColumnCoordinates( vertexId, columnPositions );

  //Apply default costs
  typename ColumnCostsContainer::Pointer columnCosts = ColumnCostsContainer::New();
  columnCosts->CreateIndex( columnPositions->Size()-1 );
  outputSurface->SetColumnCosts( vertexId, columnCosts );

  //Apply a default position at the centermost node
  outputSurface->SetInitialVertexPositionIdentifier( vertexId, 0 );
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
typename CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>::DirectionVector
CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::GetNormal(const VertexIdentifier vertexId) const
{
  DirectionVector direction;
  direction.Fill(0.0);
  using Point = typename OSFSurface::CoordinateType;

  typename OSFSurface::ConstPointer inputSurface =  this->GetInput()->GetSurface();

  Point centerPosition;
  centerPosition[0] = m_CenterPoint[0];
  centerPosition[1] = m_CenterPoint[1];
  centerPosition[2] = m_CenterPoint[2];

  //From the center point to the vertex of this column is the outward normal direction
  //Use point subtraction to get a vector from center to vertex
  Point vertexPosition = inputSurface->GetInitialVertexPosition( vertexId );

  direction[0] = vertexPosition[0] - centerPosition[0];
  direction[1] = vertexPosition[1] - centerPosition[1];
  direction[2] = vertexPosition[2] - centerPosition[2];

  
  // make compiler happy/normalize resulting vector
  if (direction.GetSquaredNorm()>0)
    direction.Normalize();
  return direction;
}



//----------------------------------------------------------------------------
//Pre-built function
template <class TInputOSFGraph, class TOutputOSFGraph>
void
CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::BuildVertexToCellLookupTable()
{
  typename OSFSurface::ConstPointer inputSurface =  this->GetInput()->GetSurface();
  typename OSFSurface::CellsContainer::ConstPointer cells = inputSurface->GetCells();
  typename OSFSurface::CellsContainer::ConstIterator cellItr = cells->Begin();
  typename OSFSurface::CellsContainer::ConstIterator cellEnd = cells->End();
 
  m_VertexToCellLookupTable.clear();
  m_VertexToCellLookupTable.resize(cells->Size());
  
  while ( cellItr!=cellEnd )
  {
    const typename OSFSurface::CellType* cellPointer = cellItr.Value();
    unsigned int numberOfPoints = cellPointer->GetNumberOfPoints();
    if (numberOfPoints>1)
    {
      typename OSFSurface::CellType::PointIdConstIterator pointIdIterator = cellPointer->PointIdsBegin();
      typename OSFSurface::CellType::PointIdConstIterator pointIdEnd = cellPointer->PointIdsEnd();
      while( pointIdIterator!=pointIdEnd )
      {
        m_VertexToCellLookupTable[ *pointIdIterator ].insert( cellItr.Index() ); 
        ++pointIdIterator;
      }
    }
      
    ++cellItr;
  }
}

//----------------------------------------------------------------------------
template <class TInputOSFGraph, class TOutputOSFGraph>
void
CenterNormalColumnBuilderFilter<TInputOSFGraph, TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif

