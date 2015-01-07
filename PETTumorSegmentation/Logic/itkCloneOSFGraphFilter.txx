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


#ifndef _itkCloneOSFGraphFilter_txx
#define _itkCloneOSFGraphFilter_txx

#include "itkCloneOSFGraphFilter.h"
#include "itkLinearInterpolateImageFunction.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TOSFGraph>
CloneOSFGraphFilter<TOSFGraph>
::CloneOSFGraphFilter()
{
}

//----------------------------------------------------------------------------
template <class TOSFGraph>
void
CloneOSFGraphFilter<TOSFGraph>
::GenerateData()
{
  this->CopyInputOSFGraphToOutputOSFGraphSurfaces();
  this->CopyInputOSFGraphToOutputOSFGraphGraph();
  this->GetOutput()->BuildGraphNodeIdentifierLookupTable();
}


//----------------------------------------------------------------------------
template <class TOSFGraph>
void
CloneOSFGraphFilter<TOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif

