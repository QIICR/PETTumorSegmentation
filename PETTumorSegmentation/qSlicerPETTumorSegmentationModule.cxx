/*==============================================================================

  Program: PETTumorSegmentation
 
  Portions (c) Copyright University of Iowa All Rights Reserved.
  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtGlobal>

// PETTumorSegmentation Logic includes
#include <vtkSlicerPETTumorSegmentationLogic.h>

// PETTumorSegmentation includes
#include "qSlicerPETTumorSegmentationModule.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerPETTumorSegmentationModule, qSlicerPETTumorSegmentationModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPETTumorSegmentationModulePrivate
{
public:
  qSlicerPETTumorSegmentationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPETTumorSegmentationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPETTumorSegmentationModulePrivate::qSlicerPETTumorSegmentationModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerPETTumorSegmentationModule methods

//-----------------------------------------------------------------------------
qSlicerPETTumorSegmentationModule::qSlicerPETTumorSegmentationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPETTumorSegmentationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPETTumorSegmentationModule::~qSlicerPETTumorSegmentationModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerPETTumorSegmentationModule::helpText() const
{
  return "This is a loadable module for tumor and lymph node segmentation in PET scans.";
}

//-----------------------------------------------------------------------------
QString qSlicerPETTumorSegmentationModule::acknowledgementText() const
{
  return "This work was funded by NIH grants U01CA140206 and U24CA180918.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPETTumorSegmentationModule::contributors() const
{
    QStringList moduleContributors;
    moduleContributors << QString("Christian Bauer (University of Iowa)");
    moduleContributors << QString("Markus van Tol (University of Iowa)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPETTumorSegmentationModule::icon() const
{
  return QIcon(":/Icons/PETTumorSegmentation.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPETTumorSegmentationModule::categories() const
{
  return QStringList() << "EditorEffect";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPETTumorSegmentationModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPETTumorSegmentationModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
bool qSlicerPETTumorSegmentationModule::isHidden() const
{
  return true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerPETTumorSegmentationModule
::createWidgetRepresentation()
{
  return 0;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPETTumorSegmentationModule::createLogic()
{
  return vtkSlicerPETTumorSegmentationLogic::New();
}
