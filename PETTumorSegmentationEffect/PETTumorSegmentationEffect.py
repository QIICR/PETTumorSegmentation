import os
from __main__ import vtk, qt, ctk, slicer
#import EditorLib
from EditorLib.EditOptions import HelpButton
#from EditorLib.EditOptions import EditOptions
from EditorLib import EditUtil
#from EditorLib import LabelEffect
#from EditorLib import Effect
from EditorLib import LabelEffectOptions, LabelEffectTool, LabelEffectLogic, LabelEffect

#
# The Editor Extension itself.
#
# This needs to define the hooks to become an editor effect.
#

#
# PETTumorSegmentationEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
# Generates the Qt GUI elements and organizes them
# Connects elements to their functionality
#
class PETTumorSegmentationEffectOptions(LabelEffectOptions):
  """ PETTumorSegmentationEffect-specfic gui
  """

  #Choose which options are used by the tool and some general setup
  #Generate the logic object for the tool
  def __init__(self, parent=0):
    super(PETTumorSegmentationEffectOptions,self).__init__(parent)
    self.usesThreshold = False; #Ignored by this tool
    self.usesPaintOver = False; #"Allow Overwriting" under Advanced replaces this
    self.undoRedoRegistered = False;

    # self.attributes should be tuple of options:
    # 'MouseTool' - grabs the cursor
    # 'Nonmodal' - can be applied while another is active
    # 'Disabled' - not available
    self.attributes = ('MouseTool')
    self.displayName = 'PET Tumor Segmentation Tool'
    self.logic = PETTumorSegmentationEffectLogic(self.editUtil.getSliceLogic())
    PETTumorSegmentationEffectLogic.options = self

  def __del__(self):
    super(PETTumorSegmentationEffectOptions,self).__del__()

  #Create the GUI for the tool
  def create(self):
    super(PETTumorSegmentationEffectOptions,self).create()

    #Save space in the GUI
    self.frame.layout().setSpacing(0)
    self.frame.layout().setMargin(0)

    # refinementBoxesFrame contains the options for how clicks are handled
    self.refinementBoxesFrame = qt.QFrame(self.frame)
    self.refinementBoxesFrame.setLayout(qt.QHBoxLayout())
    self.refinementBoxesFrame.layout().setSpacing(0)
    self.refinementBoxesFrame.layout().setMargin(0)
    self.frame.layout().addWidget(self.refinementBoxesFrame)

    #default is global refinement (threshold refinement)
    self.noRefinementRadioButton = qt.QRadioButton("Create new", self.refinementBoxesFrame)
    self.noRefinementRadioButton.setToolTip("On click, always segment a new object.")
    self.globalRefinementRadioButton = qt.QRadioButton("Global refinement", self.refinementBoxesFrame)
    self.globalRefinementRadioButton.setToolTip("On click, refine globally (adjusting then entire boundary) if no center point for the label, otherwise segment a new object.")
    self.localRefinementRadioButton = qt.QRadioButton("Local refinement", self.refinementBoxesFrame)
    self.localRefinementRadioButton.setToolTip("On click, refine locally (adjusting part of the boundary) if no center point for the label, otherwise segment a new object.")
    self.globalRefinementRadioButton.setChecked(True)

    #radio button so only one can be applied
    self.refinementBoxesFrame.layout().addWidget(qt.QLabel("Interaction style: ", self.refinementBoxesFrame))
    self.refinementBoxesFrame.layout().addWidget(self.noRefinementRadioButton)
    self.refinementBoxesFrame.layout().addWidget(self.globalRefinementRadioButton)
    self.refinementBoxesFrame.layout().addWidget(self.localRefinementRadioButton)
    self.refinementBoxesFrame.layout().addStretch(1)
    self.widgets.append(self.noRefinementRadioButton)
    self.widgets.append(self.globalRefinementRadioButton)
    self.widgets.append(self.localRefinementRadioButton)
    self.noRefinementRadioButton.connect('clicked()', self.onRefinementTypeChanged)
    self.localRefinementRadioButton.connect('clicked()', self.onRefinementTypeChanged)
    self.globalRefinementRadioButton.connect('clicked()', self.onRefinementTypeChanged)

    #options are hidden (collapsed) until requested
    self.optFrame = ctk.ctkCollapsibleButton(self.frame);
    self.optFrame.setText("Options")
    self.optFrame.setLayout(qt.QVBoxLayout())
    self.optFrame.layout().setSpacing(0)
    self.optFrame.layout().setMargin(0)
    self.optFrame.visible = True
    self.optFrame.collapsed = True
    self.optFrame.collapsedHeight = 0
    self.optFrame.setToolTip("Displays algorithm options.")

    #most useful options are kept on top: Splitting, Sealing, Assist Centering, Allow
    # Overwriting
    #to save vertical space, put 2 ina  row, so subframes here with horizontal layout are used

    #first row
    self.commonCheckBoxesFrame1 = qt.QFrame(self.optFrame)
    self.commonCheckBoxesFrame1.setLayout(qt.QHBoxLayout())
    self.commonCheckBoxesFrame1.layout().setSpacing(0)
    self.commonCheckBoxesFrame1.layout().setMargin(0)
    self.optFrame.layout().addWidget(self.commonCheckBoxesFrame1)

    #top left
    self.splittingCheckBox = qt.QCheckBox("Splitting", self.commonCheckBoxesFrame1)
    self.splittingCheckBox.setToolTip("Cut off adjacent objects to the target via watershed or local minimum.  Useful for lymph node chains.")
    self.splittingCheckBox.checked = False
    self.commonCheckBoxesFrame1.layout().addWidget(self.splittingCheckBox)
    self.widgets.append(self.splittingCheckBox)

    #top right
    self.sealingCheckBox = qt.QCheckBox("Sealing", self.commonCheckBoxesFrame1)
    self.sealingCheckBox.setToolTip("Close single-voxel gaps in the object or between the object and other objects, if above the threshold.  Useful for lymph node chains.")
    self.sealingCheckBox.checked = False
    self.commonCheckBoxesFrame1.layout().addWidget(self.sealingCheckBox)
    self.widgets.append(self.sealingCheckBox)

    #second row
    self.commonCheckBoxesFrame2 = qt.QFrame(self.optFrame)
    self.commonCheckBoxesFrame2.setLayout(qt.QHBoxLayout())
    self.commonCheckBoxesFrame2.layout().setSpacing(0)
    self.commonCheckBoxesFrame2.layout().setMargin(0)
    self.optFrame.layout().addWidget(self.commonCheckBoxesFrame2)

    #bottom left
    self.assistCenteringCheckBox = qt.QCheckBox("Assist Centering", self.commonCheckBoxesFrame2)
    self.assistCenteringCheckBox.setToolTip("Move the center to the highest voxel within 7 physical units, without being on or next to other object labels.  Improves consistency.")
    self.assistCenteringCheckBox.checked = True
    self.commonCheckBoxesFrame2.layout().addWidget(self.assistCenteringCheckBox)
    self.widgets.append(self.assistCenteringCheckBox)

    #bottom right
    self.allowOverwritingCheckBox = qt.QCheckBox("Allow Overwriting", self.commonCheckBoxesFrame2)
    self.allowOverwritingCheckBox.setToolTip("Ignore other object labels.")
    self.allowOverwritingCheckBox.checked = False
    self.commonCheckBoxesFrame2.layout().addWidget(self.allowOverwritingCheckBox)
    self.widgets.append(self.allowOverwritingCheckBox)

    #advanced options, for abnormal cases such as massive necrotic objects or
    #low-transition scans like phantoms
    #infrequently used, just keep vertical
    self.advFrame = ctk.ctkCollapsibleButton(self.optFrame);
    self.advFrame.setText("Advanced")
    self.advFrame.setLayout(qt.QVBoxLayout())
    self.advFrame.layout().setSpacing(0)
    self.advFrame.layout().setMargin(0)
    self.advFrame.visible = True
    self.advFrame.collapsed = True
    self.advFrame.collapsedHeight = 0
    self.advFrame.setToolTip("Displays more advanced algorithm options.  Do not use if you don't know what they mean.")

    #top
    self.necroticRegionCheckBox = qt.QCheckBox("Necrotic Region", self.advFrame)
    self.necroticRegionCheckBox.setToolTip("Prevents cutoff from low uptake.  Use if placing a center inside a necrotic region.")
    self.necroticRegionCheckBox.checked = False
    self.advFrame.layout().addWidget(self.necroticRegionCheckBox)
    self.widgets.append(self.necroticRegionCheckBox)

    #middle
    self.denoiseThresholdCheckBox = qt.QCheckBox("Denoise Threshold", self.advFrame)
    self.denoiseThresholdCheckBox.setToolTip("Calculates threshold based on median-filtered image.  Use only if scan is very noisey.")
    self.denoiseThresholdCheckBox.checked = False
    self.advFrame.layout().addWidget(self.denoiseThresholdCheckBox)
    self.widgets.append(self.denoiseThresholdCheckBox)

    #bottom
    self.linearCostCheckBox = qt.QCheckBox("Linear Cost", self.advFrame)
    self.linearCostCheckBox.setToolTip("Cost function below threshold is linear rather than based on region.  Use only if little/no transition region in uptake.")
    self.linearCostCheckBox.checked = False
    self.advFrame.layout().addWidget(self.linearCostCheckBox)
    self.widgets.append(self.linearCostCheckBox)

    self.optFrame.layout().addWidget(self.advFrame);

    #apply button kept at bottom of all options
    self.applyButton = qt.QPushButton("Apply", self.optFrame)
    self.optFrame.layout().addWidget(self.applyButton)
    self.applyButton.connect('clicked()', self.onApply )
    self.widgets.append(self.applyButton)
    self.applyButton.setToolTip("Redo last segmentation with the same center and refinement points with any changes in options.")

    #When changing settings, update the MRML with it
    self.assistCenteringCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.allowOverwritingCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.splittingCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.sealingCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.necroticRegionCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.denoiseThresholdCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)
    self.linearCostCheckBox.connect('toggled(bool)', self.updateMRMLFromGUI)

    self.frame.layout().addWidget(self.optFrame);

    HelpButton(self.frame, "Click on a lesion in a PET scan to segment it. Depending on refinement settings, click again to refine globally and/or locally. Options may help deal with cases such as segmenting individual lesions in a chain. For more information: http://www.slicer.org/slicerWiki/index.php/Documentation/4.4/Modules/PETTumorSegmentationEffect")

    # Add vertical spacer
    self.frame.layout().addStretch(1)

    # Clear any existing data and undo/redo queue; working directly with other tools is
    # beyond the scope of this
    self.logic.reset()

    self.updateGUIFromMRML(self,0)

  #Registers the undo/redo buttons from Slicer so that native onUndo/onRedo functions can be
  #tied to them
  def registerEditorUndoRedo(self):
    # register editor undo/redo functionality
    # due to order of object loading in Slicer, this must be done relatively late, and so is
    # kept in its own subroutine
    self.undoRedo.addUndoObserver(self.onUndo)
    self.undoRedo.addRedoObserver(self.onRedo)
    self.undoRedoRegistered = True

  #Called when removing the tool
  def destroy(self):
    super(PETTumorSegmentationEffectOptions,self).destroy()

    #hide all debug details
    self.logic.clearDebugInfo()

    #free up memory in scene undo/redo when removing tool
    self.logic.reset()

    # unregister editor undo/redo
    # This prevents our onUndo/onRedo methods from being activated when out of our tool
    # Since we can't recognize when new states are applied, we don't try to track the entire
    # undo/redo process
    self.undoRedo.removeUndoObserver(self.onUndo)
    self.undoRedo.removeRedoObserver(self.onRedo)

  # note: this method needs to be implemented exactly as-is
  # in each leaf subclass so that "self" in the observer
  # is of the correct type
  def updateParameterNode(self, caller, event):
    node = EditUtil.getParameterNode()
    if node != self.parameterNode:
      if self.parameterNode:
        node.RemoveObserver(self.parameterNodeTag)
      self.parameterNode = node
      self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

  #Updating the GUI from the MRML node keeps the options steady when leaving and returning to
  #the tool.
  #Also updates the options when undoing/redoing within the tool
  def updateGUIFromMRML(self,caller,event):
    self.updatingGUI = True

    super(PETTumorSegmentationEffectOptions,self).updateGUIFromMRML(caller,event)
    self.disconnectWidgets()

    segmentationParametersNode = self.logic.getPETTumorSegmentationParameterNode()
    if (segmentationParametersNode.GetGlobalRefinementOn()):
      self.globalRefinementRadioButton.checked = True
    elif (segmentationParametersNode.GetLocalRefinementOn()):
      self.localRefinementRadioButton.checked = True
    else:
      self.noRefinementRadioButton.checked = True
    self.assistCenteringCheckBox.checked = segmentationParametersNode.GetAssistCentering()
    self.allowOverwritingCheckBox.checked = segmentationParametersNode.GetPaintOver()
    self.splittingCheckBox.checked = segmentationParametersNode.GetSplitting()
    self.sealingCheckBox.checked = segmentationParametersNode.GetSealing()
    self.denoiseThresholdCheckBox.checked = segmentationParametersNode.GetDenoiseThreshold()
    self.linearCostCheckBox.checked = segmentationParametersNode.GetLinearCost()
    self.necroticRegionCheckBox.checked = segmentationParametersNode.GetNecroticRegion()
    self.connectWidgets()

    self.updatingGUI = False

  #Applies from the button.  Due to lack of center point, this must be certain that the tool
  #was previously used so that a center point is available.
  def onApply(self):
    #do not apply if no previous use of the tool to update settings for
    if self.logic.scene.GetNumberOfUndoLevels() == 0:
      return
    self.logic.undoRedo = self.undoRedo
    self.logic.saveStateForUndo()
    self.updateMRMLFromGUI()
    self.logic.applyParameters()


  #think we can exorcise this and attach this bit to updateMRMLFromGUI
  def onRefinementTypeChanged(self):
    if self.updatingGUI:
      return
    segmentationParametersNode = self.logic.getPETTumorSegmentationParameterNode()
    segmentationParametersNode.SetGlobalRefinementOn( self.globalRefinementRadioButton.checked )
    segmentationParametersNode.SetLocalRefinementOn( self.localRefinementRadioButton.checked )

  # set MRML node to the options in the GUI
  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    disableState = self.parameterNode.GetDisableModifiedEvent()
    self.parameterNode.SetDisableModifiedEvent(1)
    super(PETTumorSegmentationEffectOptions,self).updateMRMLFromGUI()

    segmentationParametersNode = self.logic.getPETTumorSegmentationParameterNode()
    segmentationParametersNode.SetGlobalRefinementOn( self.globalRefinementRadioButton.checked )
    segmentationParametersNode.SetLocalRefinementOn( self.localRefinementRadioButton.checked )
    segmentationParametersNode.SetAssistCentering( self.assistCenteringCheckBox.checked )
    segmentationParametersNode.SetPaintOver( self.allowOverwritingCheckBox.checked )
    segmentationParametersNode.SetSplitting( self.splittingCheckBox.checked )
    segmentationParametersNode.SetSealing( self.sealingCheckBox.checked )
    segmentationParametersNode.SetDenoiseThreshold( self.denoiseThresholdCheckBox.checked )
    segmentationParametersNode.SetLinearCost( self.linearCostCheckBox.checked )
    segmentationParametersNode.SetNecroticRegion( self.necroticRegionCheckBox.checked )

    self.parameterNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      self.parameterNode.InvokePendingModifiedEvent()

  #calls our personal undo actions in the logic
  #also shows debug info
  def onUndo(self):
    self.logic.undo()
    self.updateGUIFromMRML(self,0)  #MODIFIABLE: remove this line and options won't change when undoing
    self.logic.showDebugInfo()

  #calls our personal redo actions in the logic
  #also shows debug info
  def onRedo(self):
    self.logic.redo()
    self.updateGUIFromMRML(self,0)  #MODIFIABLE: remove this line and options won't change when undoing
    self.logic.showDebugInfo()

  def onTest(self):
    test = PETTumorSegmentationEffectWidgetTest()
    test.runTest()


#
# PETTumorSegmentationEffectTool
# Recognizes the click event and calls logic.  Multiple instances of this are generated.
#

class PETTumorSegmentationEffectTool(LabelEffectTool):
  """
  One instance of this will be created per-view when the effect
  is selected.  It is responsible for implementing feedback and
  label map changes in response to user input.
  This class observes the editor parameter node to configure itself
  and queries the current view for background and label volume
  nodes to operate on.
  """

  def __init__(self, sliceWidget):
    super(PETTumorSegmentationEffectTool,self).__init__(sliceWidget)
    self.sliceWidget = sliceWidget
    self.logic = PETTumorSegmentationEffectLogic(self.sliceWidget.sliceLogic())
  def cleanup(self):
    super(PETTumorSegmentationEffectTool,self).cleanup()

  # Handler for all events.  Only a left click is counted at the moment.
  # May in the future decide to restrict it from noting events in case of certain buttons
  # being held (shift left click to move image).
  def processEvent(self, caller=None, event=None):
    """
    handle events from the render window interactor
    """
    self.processCalled = True
    if event == "LeftButtonPressEvent":
      xy = self.interactor.GetEventPosition()
      self.logic.undoRedo = self.undoRedo
      self.logic.applyXY(xy, self.editUtil.getBackgroundID(), self.editUtil.getLabelID())
      self.abortEvent(event)
    else:
      pass


#
# PETTumorSegmentationEffectLogic
# Main methods for our effect here.
#

class PETTumorSegmentationEffectLogic(LabelEffectLogic):
  """
  This class contains helper methods for a given effect
  type.  It can be instanced as needed by an PETTumorSegmentationEffectTool
  or PETTumorSegmentationEffectOptions instance in order to compute intermediate
  results (say, for user feedback) or to implement the final
  segmentation editing operation.  This class is split
  from the PETTumorSegmentationEffectTool so that the operations can be used
  by other code without the need for a view context.
  """

  scene = slicer.vtkMRMLScene()
  vtkSegmentationLogic = None
  segmentationParameters = None
  options = None
  showDebugInfoOn = False
  undoDeficit = 0

  #Tested these as class static variables, but they weren't being appropriately static; they were inconsistent across views.
  #As such, I'm leaving them attached to the parameter node for now.
  #imageStashUndoQueue = []
  #imageStashRedoQueue = []

  def __init__(self,sliceLogic):
    super(PETTumorSegmentationEffectLogic,self).__init__(sliceLogic)
    self.sliceLogic = sliceLogic

    #undo deficit tracks undone actions of other tools, or the same tool on another stretch
    #allows matching undos whent he scene undo list fails to match the volume undo list
    #self.

    self.scene.SetUndoOn()

    #set the helper logic if needed.
    if not PETTumorSegmentationEffectLogic.vtkSegmentationLogic:
      PETTumorSegmentationEffectLogic.vtkSegmentationLogic = slicer.vtkSlicerPETTumorSegmentationLogic()
    if not PETTumorSegmentationEffectLogic.segmentationParameters:
      PETTumorSegmentationEffectLogic.segmentationParameters = self.getPETTumorSegmentationParameterNode()

    #undo/redo queue to recall the vtkImageStash.  vtkImageStash is a difficult class to set
    #up in C++, so it's handled in Python.  Maybe not the most elegant solution, but sufficient for now.
    if not hasattr(PETTumorSegmentationEffectLogic.segmentationParameters, 'imageStashUndoQueue'):
      PETTumorSegmentationEffectLogic.segmentationParameters.imageStashUndoQueue = []
    if not hasattr(PETTumorSegmentationEffectLogic.segmentationParameters, 'imageStashRedoQueue'):
      PETTumorSegmentationEffectLogic.segmentationParameters.imageStashRedoQueue = []

  # Safely return parameter node
  def getPETTumorSegmentationParameterNode(self):
    """Get the Editor parameter node - a singleton in the scene"""
    node = self._findPETTumorSegmentationParameterNodeInScene()
    if not node:
      node = self._createPETTumorSegmentationParameterNode()
    return node

  # Finds existing parameter node, if it exits
  def _findPETTumorSegmentationParameterNodeInScene(self):
    node = None
    size =  self.scene.GetNumberOfNodesByClass("vtkMRMLPETTumorSegmentationParametersNode")
    if (size>0):
      node = self.scene.GetNthNodeByClass( 0, "vtkMRMLPETTumorSegmentationParametersNode" )
    return node

  # Generates parameter node it nonexistant
  # Also adds it and the fiducial nodes to the scene
  def _createPETTumorSegmentationParameterNode(self):
    """create the PETTumorSegmentation parameter node - a singleton in the scene
    This is used internally by getPETTumorSegmentationParameterNode - shouldn't really
    be called for any other reason.
    """
    node = slicer.vtkMRMLPETTumorSegmentationParametersNode()
    node.SetUndoEnabled(True)
    self.scene.AddNode(node)
    # Since we are a singleton, the scene won't add our node into the scene,
    # but will instead insert a copy, so we find that and return it
    node = self._findPETTumorSegmentationParameterNodeInScene()

    centerFiducialList = slicer.vtkMRMLFiducialListNode()
    centerFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( centerFiducialList )
    node.SetCenterPointIndicatorListReference( centerFiducialList.GetID() )

    globalRefinementFiducialList = slicer.vtkMRMLFiducialListNode()
    globalRefinementFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( globalRefinementFiducialList )
    node.SetGlobalRefinementIndicatorListReference( globalRefinementFiducialList.GetID() )

    localRefinementFiducialList = slicer.vtkMRMLFiducialListNode()
    localRefinementFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( localRefinementFiducialList )
    node.SetLocalRefinementIndicatorListReference( localRefinementFiducialList.GetID() )

    return node

  # Clears out all queues of data.
  # Removes all fiducials from personal scene.
  def reset(self):
    # On reset, act as if all existing labels and such are now just products of other tools.
    self.scene.ClearUndoStack()
    self.scene.ClearRedoStack()

    self.segmentationParameters.Clear()
    self.segmentationParameters.imageStashUndoQueue = []
    self.segmentationParameters.imageStashRedoQueue = []

    centerPointFiducialsNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetCenterPointIndicatorListReference() ) )
    centerPointFiducials = slicer.vtkMRMLFiducialListNode.SafeDownCast( centerPointFiducialsNode )
    centerPointFiducials.RemoveAllFiducials()
    globalRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetGlobalRefinementIndicatorListReference() ) )
    globalRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( globalRefinementFiducialNode )
    globalRefinementFiducial.RemoveAllFiducials()
    localRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetLocalRefinementIndicatorListReference() ) )
    localRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( localRefinementFiducialNode )
    localRefinementFiducial.RemoveAllFiducials()
    self.undoDeficit = 0

  #Calls standard save state for label map and includes our scene save state.
  #Would include image stash save state here, but this gets called by the editor itself on Apply, so it can't take another argument.
  #So, the image stash queue is updated in line instead.
  def saveStateForUndo(self):
    if self.options and self.options.undoRedoRegistered == False:
      self.options.registerEditorUndoRedo()
    if self.undoRedo:
      self.undoRedo.saveState()
      #capture reference to the vtkImageStash used when saving the state

    self.scene.SaveStateForUndo()


  #Calls scene undo if available and handles stash queue undo.  Increments undo deficit if needed.
  def undo(self):
    #if scene has levels to undo, then undo queue is still on this tool's results
    if (self.scene.GetNumberOfUndoLevels() > 0):
      self.scene.Undo()
      #move top of stash undo queue to top of stash redo queue
      self.segmentationParameters.imageStashRedoQueue.append(self.segmentationParameters.imageStashUndoQueue[-1])
      self.segmentationParameters.imageStashUndoQueue.pop()
    #otherwise, it is undoing other tool's results, so use deficit as buffer
    else:
      self.undoDeficit += 1

  #Calls scene redo if available and handles stash queue redo.  Decrements undo deficit if needed.
  def redo(self):
    #if scene has levels to redo, then redo queue is still on this tool's results
    if (self.undoDeficit == 0):
      self.scene.Redo()
      #move top of stash redo queue to top of stash undo queue
      self.segmentationParameters.imageStashUndoQueue.append(self.segmentationParameters.imageStashRedoQueue[-1])
      self.segmentationParameters.imageStashRedoQueue.pop()
    #otherwise, it is redoing other tool's results, so use deficit as buffer
    else:
      self.undoDeficit -= 1

  def applyXY(self,xy,volumeID,labelVolumeID):
      #Convert the mouse point to a point in 3D space
      ras = self.xyToRAS(xy)
      self.apply(ras,volumeID,labelVolumeID)

  #Use the tool with a mouse click location
  #Handles all variants of refinement
  def apply(self,ras,volumeID,labelVolumeID):
      #Avoid undos across volumes by checking if the volume has changed and resetting if so
      newLabel = self.editUtil.getLabel()
      newVolumeID = volumeID
      newLabelVolumeID = labelVolumeID
      oldLabel = self.segmentationParameters.GetLabel()
      oldVolumeID = self.segmentationParameters.GetPETVolumeReference()
      oldLabelVolumeID = self.segmentationParameters.GetSegmentationVolumeReference()
      #Do so before saving the state
      if (newVolumeID != oldVolumeID or newLabelVolumeID != oldLabelVolumeID  ):
        self.reset()

      #State is saved at other points, but the stash only needs to be acquired when placing a center point,
      #so it's fine to leave out it's state save equivalent from saveStateForUndo.
      self.saveStateForUndo()
      if (self.options):
        self.options.updateMRMLFromGUI()

      #Get the fiducial nodes
      centerPointFiducialsNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetCenterPointIndicatorListReference() ) )
      centerPointFiducials = slicer.vtkMRMLFiducialListNode.SafeDownCast( centerPointFiducialsNode )
      globalRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetGlobalRefinementIndicatorListReference() ) )
      globalRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( globalRefinementFiducialNode )
      localRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetLocalRefinementIndicatorListReference() ) )
      localRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( localRefinementFiducialNode )

      #Set the current label and volumes
      self.segmentationParameters.SetLabel( self.editUtil.getLabel() )
      self.segmentationParameters.SetPETVolumeReference( volumeID )
      self.segmentationParameters.SetSegmentationVolumeReference( labelVolumeID )

      #If there are no center points, then the click should make a new center point and segmentation.
      #If the label has changed, go with this and remove center points to start a new segmentation.
      #If no refinement mode is on, remove points and start a new segmentation.
      if (centerPointFiducials.GetNumberOfFiducials()==0 or newLabel != oldLabel or self.segmentationParameters.GetNoRefinementOn()): # initialize segmentation
        #clear old fiducials if applying new due to No Refinement or new label
        if (centerPointFiducials.GetNumberOfFiducials()!=0):
          globalRefinementFiducial.RemoveAllFiducials()
          localRefinementFiducial.RemoveAllFiducials()
          centerPointFiducials.RemoveAllFiducials()
        #add the new center point
        centerPointFiducials.AddFiducialWithXYZ(ras[0], ras[1], ras[2], False )

        #Pre-tool state is directly previous in the Editor's label map queue.
        imageStash = self.undoRedo.undoList[-1].stash

        #Copy it as current pre-tool state into the stash undo queue
        self.segmentationParameters.imageStashUndoQueue.append(imageStash)
        self.segmentationParameters.imageStashRedoQueue = []

        #Decompress if for use.  Waiting loop required to check that previous Stash action complete.
        while (imageStash.GetStashing() != 0):
          pass
        imageStash.Unstash()

        #Apply new segmentation with parameters and pre-tool label image state
        self.vtkSegmentationLogic.Apply( self.segmentationParameters, imageStash.GetStashImage() )

        #Re-compress stashed image in queues
        imageStash.ThreadedStash()
      #Otherwise, refining, either globally or locally.
      else:
        #Pre-tool state is the same as it was for the previous use of the tool, so it's at the top of the stash undo queue.
        imageStash = self.segmentationParameters.imageStashUndoQueue[-1]

        #Copy it as current pre-tool state into the stash undo queue
        self.segmentationParameters.imageStashUndoQueue.append(imageStash)
        self.segmentationParameters.imageStashRedoQueue = []


        #Decompress if for use.  Waiting loop required to check that previous Stash action complete.
        while (imageStash.GetStashing() != 0):
          pass
        imageStash.Unstash()

        #If global refinement is active, then the existing segmentation should be refined globally.
        if (self.segmentationParameters.GetGlobalRefinementOn()): # perform global refinement, unless new label
          #Only one global refinement point at a time
          globalRefinementFiducial.RemoveAllFiducials()
          globalRefinementFiducial.AddFiducialWithXYZ(ras[0], ras[1], ras[2], False )
          self.vtkSegmentationLogic.ApplyGlobalRefinement( self.segmentationParameters, imageStash.GetStashImage() )
        #If local refinement is active, then the existing segmentation should be refined localy.
        elif (self.segmentationParameters.GetLocalRefinementOn()): # perform local refinement, unless new label
          localRefinementFiducial.AddFiducialWithXYZ(ras[0], ras[1], ras[2], False )
          self.vtkSegmentationLogic.ApplyLocalRefinement( self.segmentationParameters, imageStash.GetStashImage() )
        #end refinement choice

        #Re-compress stashed image in queues
        imageStash.ThreadedStash()
      #end check if new point is new segmentation or a refinement point

      #clear any undo deficit due to cutting off redo queues
      self.undoDeficit = 0

      #This is already done in the C++ logic, so it is left out here.
      #labelVolumeNode = slicer.mrmlScene.GetNodeByID(str(labelVolumeID))
      #labelVolume = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(labelVolumeNode)
      #labelVolume.GetImageData().Modified()
      #labelVolume.Modified()
      self.showDebugInfo()

  def applyParameters(self):
      #saveStateForUndo() # this has to be called by editor before the MRML node is updated

      centerPointFiducialsNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetCenterPointIndicatorListReference() ) )
      centerPointFiducials = slicer.vtkMRMLFiducialListNode.SafeDownCast( centerPointFiducialsNode )

      self.segmentationParameters.SetLabel( self.editUtil.getLabel() )
      self.segmentationParameters.SetPETVolumeReference( self.editUtil.getBackgroundID() )
      self.segmentationParameters.SetSegmentationVolumeReference( self.editUtil.getLabelID() )

      # Apply from button acts like refinement, so it shares the same pre-tool state, on top of the stash undo queue
      imageStash = self.segmentationParameters.imageStashUndoQueue[-1]

      #Copy it as current pre-tool state into the stash undo queue
      self.segmentationParameters.imageStashUndoQueue.append(imageStash)
      self.segmentationParameters.imageStashRedoQueue = []

      #Decompress if for use.  Waiting loop required to check that previous Stash action complete.
      while (imageStash.GetStashing() != 0):
        pass
      imageStash.Unstash()

      # start segmention with updated parameters
      self.vtkSegmentationLogic.Apply( self.segmentationParameters, imageStash.GetStashImage() )

      #Re-compress stashed image in queues
      imageStash.ThreadedStash()

      #This is already done in the C++ logic, so it is left out here.
      #labelVolumeNode = slicer.mrmlScene.GetNodeByID(str(self.editUtil.getLabelID()))
      #labelVolume = slicer.vtkMRMLScalarVolumeNode.SafeDownCast(labelVolumeNode)
      #labelVolume.GetImageData().Modified()
      #labelVolume.Modified()
      self.showDebugInfo()

  #Shows the center point and refinement points of the current scene state.
  #Creates a unique fiducials node for it.
  def showDebugInfo(self):
      if (not self.showDebugInfoOn):
        return False;

      #Check for existance of our fiducial list.  If it exists, use it as is.
      #Possible bug: If someone generates a list by this name, it could conflict with ours.
      #...Need to make it static and delete it on destruction of our tool object to be safe.
      #It doesn't properly hide from editors
      osfDebugFiducialsNodes = slicer.mrmlScene.GetNodesByName("OSFDebugFiducials")
      osfDebugFiducialsNodes.SetReferenceCount(osfDebugFiducialsNodes.GetReferenceCount()-1) #Prevents memory leaks from GetNodesByName
      osfDebugFiducialsNode = osfDebugFiducialsNodes.GetItemAsObject(0)
      osfDebugFiducials = None
      if (osfDebugFiducialsNode):
        osfDebugFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast(osfDebugFiducialsNode)
        osfDebugFiducials.RemoveAllMarkups()
      else:
        markupsLogic = slicer.modules.markups.logic()
        osfDebugFiducialsID = markupsLogic.AddNewFiducialNode("OSFDebugFiducials", slicer.mrmlScene)
        osfDebugFiducials = slicer.mrmlScene.GetNodeByID(osfDebugFiducialsID)
      osfDebugFiducials.SetHideFromEditors(True)

      centerPointFiducialsNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetCenterPointIndicatorListReference() ) )
      centerPointFiducials = slicer.vtkMRMLFiducialListNode.SafeDownCast( centerPointFiducialsNode )
      globalRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetGlobalRefinementIndicatorListReference() ) )
      globalRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( globalRefinementFiducialNode )
      localRefinementFiducialNode = self.scene.GetNodeByID( str( self.segmentationParameters.GetLocalRefinementIndicatorListReference() ) )
      localRefinementFiducial = slicer.vtkMRMLFiducialListNode.SafeDownCast( localRefinementFiducialNode )
      if (centerPointFiducials.GetNumberOfFiducials()>0):
        centerXYZ = [self.segmentationParameters.GetCenterpointX(), self.segmentationParameters.GetCenterpointY(), self.segmentationParameters.GetCenterpointZ()]
        centerXYZ[0] = -centerXYZ[0]
        centerXYZ[1] = -centerXYZ[1]
        centerInt = osfDebugFiducials.AddFiducialFromArray(centerXYZ)
        osfDebugFiducials.SetNthFiducialLabel(centerInt, "C")
        osfDebugFiducials.SetNthMarkupLocked(centerInt, True)
        osfDebugFiducials.SetNthFiducialVisibility(centerInt, True)
      if (globalRefinementFiducial.GetNumberOfFiducials()>0):
        thresholdInt = osfDebugFiducials.AddFiducialFromArray(globalRefinementFiducial.GetNthFiducialXYZ(globalRefinementFiducial.GetNumberOfFiducials()-1))
        osfDebugFiducials.SetNthFiducialLabel(thresholdInt, "Th")
        osfDebugFiducials.SetNthMarkupLocked(thresholdInt, True)
        osfDebugFiducials.SetNthFiducialVisibility(thresholdInt, True)
      if (localRefinementFiducial.GetNumberOfFiducials()>0):
        for i in range(localRefinementFiducial.GetNumberOfFiducials()):
          edgeInt = osfDebugFiducials.AddFiducialFromArray(localRefinementFiducial.GetNthFiducialXYZ(i))
          osfDebugFiducials.SetNthFiducialLabel(edgeInt, "Ed"+str(i))
          osfDebugFiducials.SetNthMarkupLocked(edgeInt, True)
          osfDebugFiducials.SetNthFiducialVisibility(edgeInt, True)

  #DEBUG ONLY
  def clearDebugInfo(self):
      if (not self.showDebugInfoOn):
        return False;
      osfDebugFiducialsNodes = slicer.mrmlScene.GetNodesByName("OSFDebugFiducials")
      osfDebugFiducialsNodes.SetReferenceCount(osfDebugFiducialsNodes.GetReferenceCount()-1)
      osfDebugFiducialsNode = osfDebugFiducialsNodes.GetItemAsObject(0)
      osfDebugFiducials = None
      if (osfDebugFiducialsNode):
        osfDebugFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast(osfDebugFiducialsNode)
        osfDebugFiducials.RemoveAllMarkups()


# The PETTumorSegmentationEffectExtension class definition
#

class PETTumorSegmentationEffectExtension(LabelEffect):
  """Organizes the Options, Tool, and Logic classes into a single instance
  that can be managed by the EditBox
  """

  def __init__(self):
    # name is used to define the name of the icon image resource (e.g. PETTumorSegmentationEffect.png)
    self.name = "PETTumorSegmentationEffect"
    # tool tip is displayed on mouse hover
    self.toolTip = "PETTumorSegmentationEffect: Use Optimal Surface Finding to segment a region; designed for PET segmentation"

    self.options = PETTumorSegmentationEffectOptions
    self.tool = PETTumorSegmentationEffectTool
    self.logic = PETTumorSegmentationEffectLogic

#
# PETTumorSegmentationEffect
#

class PETTumorSegmentationEffect(object):
  """
  This class is the 'hook' for slicer to detect and recognize the extension
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "Editor PETTumorSegmentationEffect Effect"
    parent.categories = ["Developer Tools.Editor Extensions"]
    parent.contributors = ["Christian Bauer (University of Iowa), Markus van Tol (University of Iowa), Steve Pieper (Isomics)"] # insert your name in the list
    parent.helpText = """
    PET Tumor Segmentation Module
    """
    parent.acknowledgementText = """
    This editor extension was developed by
    Christian Bauer, University of Iowa and
    Markus van Tol, Univesity of Iowa
    based on work by:
    Steve Pieper, Isomics, Inc.
    based on work by:
    Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grants U01CA140206, U24CA180918 and 3P41RR013218.
    """

    # don't show this module - it only appears in the Editor module
    parent.hidden = True

    # Add this extension to the editor's list for discovery when the module
    # is created.  Since this module may be discovered before the Editor itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.editorExtensions
    except AttributeError:
      slicer.modules.editorExtensions = {}
    slicer.modules.editorExtensions['PETTumorSegmentationEffect'] = PETTumorSegmentationEffectExtension

#
# PETTumorSegmentationEffectWidget
#

class PETTumorSegmentationEffectWidget(object):
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass
