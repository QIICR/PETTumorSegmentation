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

#ifndef _itkOSFGraphSource_txx
#define _itkOSFGraphSource_txx

#include "itkOSFGraph.h"

namespace itk
{

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
OSFGraphSource<TOutputOSFGraph>
::OSFGraphSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputMesh
  OutputOSFGraphPointer output
    = static_cast<TOutputOSFGraph*>(this->MakeOutput(0).GetPointer());

  this->ProcessObject::SetNumberOfRequiredOutputs(1);
  this->ProcessObject::SetNthOutput( 0, output.GetPointer() );
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
typename OSFGraphSource<TOutputOSFGraph>::DataObjectPointer
OSFGraphSource<TOutputOSFGraph>
::MakeOutput(DataObjectPointerArraySizeType)
{
  return static_cast<DataObject*>(TOutputOSFGraph::New().GetPointer());
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
typename OSFGraphSource<TOutputOSFGraph>::OutputOSFGraphType*
OSFGraphSource<TOutputOSFGraph>
::GetOutput(void)
{
  if (this->GetNumberOfOutputs() < 1)
    {
    return 0;
    }

  return static_cast<TOutputOSFGraph*>
    (this->ProcessObject::GetOutput(0));
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
typename OSFGraphSource<TOutputOSFGraph>::OutputOSFGraphType*
OSFGraphSource<TOutputOSFGraph>
::GetOutput(unsigned int idx)
{
  return static_cast<TOutputOSFGraph*>
    (this->ProcessObject::GetOutput(idx));
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
void
OSFGraphSource<TOutputOSFGraph>
::SetOutput(OutputOSFGraphType *output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  this->ProcessObject::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
void
OSFGraphSource<TOutputOSFGraph>
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
void
OSFGraphSource<TOutputOSFGraph>
::GraftOutput(DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
void
OSFGraphSource<TOutputOSFGraph>
::GraftNthOutput(unsigned int idx, DataObject *graft)
{
  if ( idx >= this->GetNumberOfOutputs() )
    {
    itkExceptionMacro(<<"Requested to graft output " << idx <<
        " but this filter only has " << this->GetNumberOfOutputs() << " Outputs.");
    }

  if ( !graft )
    {
    itkExceptionMacro(<<"Requested to graft output that is a nullptr pointer" );
    }

  DataObject * output = this->GetOutput( idx );

  // Call Graft on the OSFGraph in order to copy meta-information, and containers.
  // todo: OSFGraph needs support for that!!!
  output->Graft( graft );
}

//----------------------------------------------------------------------------
template <class TOutputOSFGraph>
void
OSFGraphSource<TOutputOSFGraph>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  // todo: implement
}

} // namespace

#endif
