import os
import unittest
import dicom
import vtk, qt, ctk, slicer, logging
from DICOMLib import DICOMUtils
from SegmentStatistics import SegmentStatisticsLogic
import vtkSegmentationCorePython as vtkSegmentationCore
from slicer.ScriptedLoadableModule import *

#
# SegmentEditorPETTumorSegmentationEffectSelfTest
#

class SegmentEditorPETTumorSegmentationEffectSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "SegmentEditorPETTumorSegmentationEffectSelfTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = ["SegmentStatistics"]
    self.parent.contributors = ["Christian Bauer (University of Iowa)"]
    self.parent.helpText = """This is a self test for SegmentEditorPETTumorSegmentationEffect."""
    parent.acknowledgementText = """This work was partially funded by NIH grants U01-CA140206 and U24-CA180918."""

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SegmentEditorPETTumorSegmentationEffectSelfTest'] = self.runTest

  def runTest(self):
    tester = SegmentEditorPETTumorSegmentationEffectSelfTestTest()
    tester.runTest()

#
# SegmentEditorPETTumorSegmentationEffectSelfTestWidget
#

class SegmentEditorPETTumorSegmentationEffectSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# SegmentEditorPETTumorSegmentationEffectSelfTestLogic
#

class SegmentEditorPETTumorSegmentationEffectSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class SegmentEditorPETTumorSegmentationEffectSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SegmentEditorEffect()
    self.tearDown()

  def setUp(self):
    """ Open temporary DICOM database
    """
    slicer.mrmlScene.Clear(0)
    self.delayMs = 700
    self.tempDataDir = os.path.join(slicer.app.temporaryPath,'PETTest')
    self.tempDicomDatabaseDir = os.path.join(slicer.app.temporaryPath,'PETTestDicom')

  def doCleanups(self):
    """ cleanup temporary data in case an exception occurs
    """
    self.tearDown()

  def tearDown(self):
    """ Close temporary DICOM database and remove temporary data
    """ 
    try:
      import shutil
      if os.path.exists(self.tempDataDir):
        shutil.rmtree(self.tempDataDir)
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e),self.delayMs*2)

  def loadTestData(self):    
    #download data and add to dicom database
    zipFileUrl = 'http://slicer.kitware.com/midas3/download/item/257234/QIN-HEADNECK-01-0139-PET.zip'
    zipFilePath = self.tempDataDir+'/dicom.zip'
    zipFileData = self.tempDataDir+'/dicom'
    expectedNumOfFiles = 545
    if not os.access(self.tempDataDir, os.F_OK):
      os.mkdir(self.tempDataDir)
    if not os.access(zipFileData, os.F_OK):
      os.mkdir(zipFileData)

    dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
    dicomPluginCheckbox =  dicomWidget.detailsPopup.pluginSelector.checkBoxByPlugin
    dicomPluginStates = {(key,value.checked) for key,value in dicomPluginCheckbox.items()}
    for cb in dicomPluginCheckbox.values(): cb.checked=False
    dicomPluginCheckbox['DICOMScalarVolumePlugin'].checked = True

    # Download, unzip, import, and load data. Verify loaded nodes.
    loadedNodes = {'vtkMRMLScalarVolumeNode':1}
    with DICOMUtils.LoadDICOMFilesToDatabase(zipFileUrl, zipFilePath, zipFileData, expectedNumOfFiles, {}, loadedNodes) as success:
      self.assertTrue(success)
      print ('loading returned true')

    self.assertEqual( len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') ), 1 )
    imageNode = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLScalarVolumeNode')

    for key,value in dicomPluginStates:
      dicomPluginCheckbox[key].checked=value

    # apply the SUVbw conversion factor and set units and quantity
    suvNormalizationFactor = 0.00040166400000000007
    quantity = slicer.vtkCodedEntry()
    quantity.SetFromString('CodeValue:126400|CodingSchemeDesignator:DCM|CodeMeaning:Standardized Uptake Value')
    units = slicer.vtkCodedEntry()
    units.SetFromString('CodeValue:{SUVbw}g/ml|CodingSchemeDesignator:UCUM|CodeMeaning:Standardized Uptake Value body weight')
    multiplier = vtk.vtkImageMathematics()
    multiplier.SetOperationToMultiplyByK()
    multiplier.SetConstantK(suvNormalizationFactor)
    multiplier.SetInput1Data(imageNode.GetImageData())
    multiplier.Update()
    imageNode.GetImageData().DeepCopy(multiplier.GetOutput())
    imageNode.GetVolumeDisplayNode().SetWindowLevel(6,3)
    imageNode.GetVolumeDisplayNode().SetAndObserveColorNodeID('vtkMRMLColorTableNodeInvertedGrey')
    imageNode.SetVoxelValueQuantity(quantity)
    imageNode.SetVoxelValueUnits(units)

    return imageNode
  
  def test_SegmentEditorEffect(self):
  
    self.assertIsNotNone( slicer.vtkSlicerPETTumorSegmentationLogic )
    with DICOMUtils.TemporaryDICOMDatabase(self.tempDicomDatabaseDir) as db:
      self.assertTrue(db.isOpen)
      self.assertEqual(slicer.dicomDatabase, db)
      
      self.delayDisplay('Loading PET DICOM dataset (including download if necessary)')
      petNode = self.loadTestData()
      self.delayDisplay('Switching to Segment Editor')
      slicer.util.mainWindow().moduleSelector().selectModule('SegmentEditor')
      widget = slicer.modules.SegmentEditorWidget
      editor = widget.editor
      params = widget.parameterSetNode
      seg = widget.editor.segmentationNode()
    
      self.delayDisplay('Adding emtpy segment')
      # add segment similar to onAddSegment: https://github.com/Slicer/Slicer/blob/28ea2ebef031788d706a3085a46fac41d0017c05/Modules/Loadable/Segmentations/Widgets/qMRMLSegmentEditorWidget.cxx#L1894
      editor.saveStateForUndo()
      addedSegmentID = seg.GetSegmentation().AddEmptySegment()
      editor.setCurrentSegmentID(addedSegmentID)
    
      # switch to PET tumor segmentation effect
      self.delayDisplay('Activating PET Tumor Segmentation effect')
      editor.setActiveEffectByName('PET Tumor Segmentation')
      effect = editor.activeEffect()
      self.assertIsNotNone(effect)
    
      # one-click segmentation
      self.delayDisplay('Applying ne-click segmentation')
      effect.self().onApplyMouseClick([41.3,220.1,-980.2])
      self.assertEqual(self.getSignature(seg),2890720391)
    
      # global refinement
      self.delayDisplay('Applying global refinement')
      effect.self().globalRefinementRadioButton.click()
      effect.self().onApplyMouseClick([40.1,264.2,-969.2])
      self.assertEqual(self.getSignature(seg),3700505133)
    
      # local refinement
      self.delayDisplay('Applying local refinement')
      effect.self().localRefinementRadioButton.click()
      effect.self().onApplyMouseClick([40.1,258.1,-1025.1])
      self.assertEqual(self.getSignature(seg),3550788737)
    
      # undo with local refinement
      self.delayDisplay('Testing undo with local refinement')
      editor.undo()
      self.assertEqual(self.getSignature(seg),3700505133)
      effect.self().onApplyMouseClick([40.1,234.2, 934.7])
    
      # go back to initial segmentation before refinement and use splitting option
      self.delayDisplay('Applying splitting option')
      editor.undo()
      editor.undo()
      effect.self().splittingCheckBox.checked = True
      effect.self().onApplyParameters()
      self.assertEqual(self.getSignature(seg),4231292517)
    
      # add second segment
      self.delayDisplay('Adding second segment')
      editor.saveStateForUndo()
      addedSegmentID = seg.GetSegmentation().AddEmptySegment()
      editor.setCurrentSegmentID(addedSegmentID)
    
      # one-click segmentation
      self.delayDisplay('Applying one-click segmentation')
      effect.self().onApplyMouseClick([41.3,229.1,-952.5])
      effect.self().onApplyParameters()
      self.assertEqual(self.getSignature(seg),4226190476)
    
      # assist centering
      self.delayDisplay('Applying assist centering option')
      effect.self().assistCenteringCheckBox.checked = False
      effect.self().onApplyParameters()
      self.assertEqual(self.getSignature(seg),4401777778)
    
      # sealing centering
      self.delayDisplay('Applying sealing option')
      effect.self().sealingCheckBox.checked = True
      effect.self().onApplyParameters()
      self.assertEqual(self.getSignature(seg),4451555556)
    
      # overwriting
      self.delayDisplay('Applying overwriting option')
      effect.self().allowOverwritingCheckBox.checked = True
      effect.self().onApplyParameters()
      self.assertEqual(self.getSignature(seg),5756378601)
    
      editor.undo()
      self.assertEqual(self.getSignature(seg),4451555556)      
    
      self.delayDisplay('Test passed!')

  def getSignature(self, seg):
    if not 'labelmap' in self.__dict__ or not self.labelmap:
      self.labelmap = slicer.vtkMRMLLabelMapVolumeNode()
      slicer.mrmlScene.AddNode(self.labelmap)
    slicer.modules.segmentations.logic().ExportAllSegmentsToLabelmapNode(seg, self.labelmap)
    stat = vtk.vtkImageAccumulate()
    stat.SetInputData(self.labelmap.GetImageData())
    stat.Update()
    return int(round(stat.GetMean()[0]*1e10))
    
