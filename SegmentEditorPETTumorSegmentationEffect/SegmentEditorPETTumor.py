import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *
from slicer.ScriptedLoadableModule import *

class SegmentEditorPETTumorEffect(AbstractScriptedSegmentEditorEffect):
  """ PETTumorEffect is an Effect that implements the
      PET Tumor segmentation in segment editor
  """

  scene = slicer.vtkMRMLScene()
  vtkSegmentationLogic = None # note: The necessary C++ module might not have been loaded yet. vtkSlicerPETTumorSegmentationLogic()

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'PET Tumor Segmentation'
    # Indicates that effect does not operate on one segment, but the whole segmentation.
    # This means that while this effect is active, no segment can be selected
    scriptedEffect.perSegment = False
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

    # Observation for auto-update
    self.observedSegmentation = None
    self.segmentationNodeObserverTags = []

    # undo/redo helpers
    self.scene.SetUndoOn()
    self.segmentationIdCounter = 0

    self.active = False
    self.debug = False

  #def __del__(self):, scriptedEffect):
  #  super(SegmentEditorPETTumorEffect,self).__del__()
  #  self.observeSegmentation(False)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'PETTumorEffect.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return "Segment tumors and hot lymph nodes in PET scans."

  def setupOptionsFrame(self):
    self.frame = qt.QFrame(self.scriptedEffect.optionsFrame())
    qt.QFormLayout(self.frame)
    #Save space in the GUI
    self.frame.layout().setSpacing(0)
    self.frame.layout().setMargin(0)

#    self.helpLabel = qt.QLabel(self.frame)
#    self.helpLabel.setWordWrap(True)
#    #self.helpLabel.sizePolicy.setHorizontalPolicy(qt.QSizePolicy.Ignored)
#    #self.helpLabel.sizePolicy.setVerticalPolicy(qt.QSizePolicy.Ignored)
#    self.helpLabel.text = "Click on a lesion to start segmentation. \
# Depending on refinement settings, click again to refine globally and/or locally. Options may help deal \
# with cases such as segmenting individual lesions in a chain. \
# See <a href=\"https://www.slicer.org/wiki/Documentation/Nightly/Extensions/PETTumorSegmentation\">the documentation</a> for more information."
#    self.scriptedEffect.addOptionsWidget(self.helpLabel)

    # refinementBoxesFrame contains the options for how clicks are handled
    self.refinementBoxesFrame = qt.QFrame(self.frame)
    self.refinementBoxesFrame.setLayout(qt.QHBoxLayout())
    self.refinementBoxesFrame.layout().setSpacing(0)
    self.refinementBoxesFrame.layout().setMargin(0)

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

    self.scriptedEffect.addOptionsWidget(self.refinementBoxesFrame)

    #options are hidden (collapsed) until requested
    self.optFrame = ctk.ctkCollapsibleGroupBox(self.frame)
    self.optFrame.setTitle("Options")
    self.optFrame.setLayout(qt.QVBoxLayout())
    self.optFrame.visible = True
    self.optFrame.collapsed = True
    self.optFrame.setToolTip("Displays algorithm options.")

    #most useful options are kept on top: Splitting, Sealing, Assist Centering, Allow
    #Overwriting; to save vertical space, put 2 in a row, so subframes here with
    #horizontal layout are used

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

    #top right
    self.sealingCheckBox = qt.QCheckBox("Sealing", self.commonCheckBoxesFrame1)
    self.sealingCheckBox.setToolTip("Close single-voxel gaps in the object or between the object and other objects, if above the threshold.  Useful for lymph node chains.")
    self.sealingCheckBox.checked = False
    self.commonCheckBoxesFrame1.layout().addWidget(self.sealingCheckBox)

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

    #bottom right
    self.allowOverwritingCheckBox = qt.QCheckBox("Allow Overwriting", self.commonCheckBoxesFrame2)
    self.allowOverwritingCheckBox.setToolTip("Ignore other object labels.")
    self.allowOverwritingCheckBox.checked = False
    self.commonCheckBoxesFrame2.layout().addWidget(self.allowOverwritingCheckBox)

    self.scriptedEffect.addOptionsWidget(self.optFrame)

    #advanced options, for abnormal cases such as massive necrotic objects or
    #low-transition scans like phantoms
    #infrequently used, just keep vertical
    self.advFrame = ctk.ctkCollapsibleGroupBox(self.frame);
    self.advFrame.setTitle("Advanced")
    self.advFrame.setLayout(qt.QVBoxLayout())
    self.advFrame.visible = True
    self.advFrame.collapsed = True
    self.advFrame.setToolTip("Displays more advanced algorithm options.  Do not use if you don't know what they mean.")

    #top
    self.necroticRegionCheckBox = qt.QCheckBox("Necrotic Region", self.advFrame)
    self.necroticRegionCheckBox.setToolTip("Prevents cutoff from low uptake.  Use if placing a center inside a necrotic region.")
    self.necroticRegionCheckBox.checked = False
    self.advFrame.layout().addWidget(self.necroticRegionCheckBox)

    #middle
    self.denoiseThresholdCheckBox = qt.QCheckBox("Denoise Threshold", self.advFrame)
    self.denoiseThresholdCheckBox.setToolTip("Calculates threshold based on median-filtered image.  Use only if scan is very noisey.")
    self.denoiseThresholdCheckBox.checked = False
    self.advFrame.layout().addWidget(self.denoiseThresholdCheckBox)

    #bottom
    self.linearCostCheckBox = qt.QCheckBox("Linear Cost", self.advFrame)
    self.linearCostCheckBox.setToolTip("Cost function below threshold is linear rather than based on region.  Use only if little/no transition region in uptake.")
    self.linearCostCheckBox.checked = False
    self.advFrame.layout().addWidget(self.linearCostCheckBox)

    self.optFrame.layout().addWidget(self.advFrame)

    #apply button kept at bottom of all options
    self.applyButton = qt.QPushButton("Apply", self.frame)
    self.optFrame.layout().addWidget(self.applyButton)
    self.applyButton.connect('clicked()', self.onApplyParameters )
    self.applyButton.setToolTip("Redo last segmentation with the same center and refinement points with any changes in options.")

  def setMRMLDefaults(self):
    self.scriptedEffect.setParameterDefault("SegmentationId", 0)

  def updateGUIFromMRML(self):
    self.updatingGUI = True

    #super(PETTumorSegmentationEffectOptions,self).updateGUIFromMRML(caller,event)

    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    if not segmentationParameters: return
    self._showDebugInfo(segmentationParameters)
    if (segmentationParameters.GetGlobalRefinementOn()):
      self.globalRefinementRadioButton.checked = True
    elif (segmentationParameters.GetLocalRefinementOn()):
      self.localRefinementRadioButton.checked = True
    else:
      self.noRefinementRadioButton.checked = True
    self.assistCenteringCheckBox.checked = segmentationParameters.GetAssistCentering()
    self.allowOverwritingCheckBox.checked = segmentationParameters.GetPaintOver()
    self.splittingCheckBox.checked = segmentationParameters.GetSplitting()
    self.sealingCheckBox.checked = segmentationParameters.GetSealing()
    self.denoiseThresholdCheckBox.checked = segmentationParameters.GetDenoiseThreshold()
    self.linearCostCheckBox.checked = segmentationParameters.GetLinearCost()
    self.necroticRegionCheckBox.checked = segmentationParameters.GetNecroticRegion()

    self.updatingGUI = False

  def updateMRMLFromGUI(self):
    if self.updatingGUI:
      return
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()
    disableState = segmentationNode.GetDisableModifiedEvent()
    segmentationNode.SetDisableModifiedEvent(1)
    #super(SegmentEditorPETTumorEffect,self).updateMRMLFromGUI()

    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    segmentationParameters.SetGlobalRefinementOn( self.globalRefinementRadioButton.checked )
    segmentationParameters.SetLocalRefinementOn( self.localRefinementRadioButton.checked )
    segmentationParameters.SetAssistCentering( self.assistCenteringCheckBox.checked )
    segmentationParameters.SetPaintOver( self.allowOverwritingCheckBox.checked )
    segmentationParameters.SetSplitting( self.splittingCheckBox.checked )
    segmentationParameters.SetSealing( self.sealingCheckBox.checked )
    segmentationParameters.SetDenoiseThreshold( self.denoiseThresholdCheckBox.checked )
    segmentationParameters.SetLinearCost( self.linearCostCheckBox.checked )
    segmentationParameters.SetNecroticRegion( self.necroticRegionCheckBox.checked )

    segmentationNode.SetDisableModifiedEvent(disableState)
    if not disableState:
      segmentationNode.InvokePendingModifiedEvent()

  def reset(self):
    self.scene.Clear(True)
    self.segmentationIdCounter = 0

    # clear PETTumorSegmentation.SegmentationId in all segments
    paramsNode = self.scriptedEffect.parameterSetNode()
    segmentation = paramsNode.GetSegmentationNode().GetSegmentation()
    segmentIDs = vtk.vtkStringArray()
    segmentation.GetSegmentIDs(segmentIDs)
    for index in range(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      segment = segmentation.GetSegment(segmentID)
      if segment.HasTag('PETTumorSegmentation.SegmentationId'):
        segment.RemoveTag('PETTumorSegmentation.SegmentationId')

  def onSegmentationModified(self, caller, event):
    if not self.active:
      self.reset()

  def observeSegmentation(self, observationEnabled):
    import vtkSegmentationCorePython as vtkSegmentationCore
    segmentation = self.scriptedEffect.parameterSetNode().GetSegmentationNode().GetSegmentation()
    if observationEnabled and self.observedSegmentation == segmentation:
      return
    if not observationEnabled and not self.observedSegmentation:
      return
    # Need to update the observer
    # Remove old observer
    if self.observedSegmentation:
      for tag in self.segmentationNodeObserverTags:
        self.observedSegmentation.RemoveObserver(tag)
      self.segmentationNodeObserverTags = []
      self.observedSegmentation = None
    # Add new observer
    if observationEnabled and segmentation is not None:
      self.observedSegmentation = segmentation
      observedEvents = [
        vtkSegmentationCore.vtkSegmentation.SegmentAdded,
        vtkSegmentationCore.vtkSegmentation.SegmentRemoved,
        vtkSegmentationCore.vtkSegmentation.SegmentModified]
      for eventId in observedEvents:
        self.segmentationNodeObserverTags.append(self.observedSegmentation.AddObserver(eventId, self.onSegmentationModified))

  def activate(self):
    self.active = True
    self.reset()
    self.observeSegmentation(True)

  def deactivate(self):
    self.active = False
    self.reset()
    self.observeSegmentation(False)

  def processInteractionEvents(self, callerInteractor, eventId, viewWidget):
    abortEvent = False

    # Only allow for slice views
    if viewWidget.className() != "qMRMLSliceWidget":
      return abortEvent

    if eventId == vtk.vtkCommand.LeftButtonPressEvent:
      xy = callerInteractor.GetEventPosition()
      rasCoorinate = self.xyToRas(xy, viewWidget)
      self.onApplyMouseClick(rasCoorinate)
      abortEvent = True
    else: # todo: add other events for keypress, mouse move?
      pass

    return abortEvent

  def saveStateForUndo(self):
    self.scriptedEffect.saveStateForUndo()
    self.scene.SaveStateForUndo()

  def onApplyMouseClick(self, rasCoorinate):
    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    if not segmentationParameters: return

    paramsNode = self.scriptedEffect.parameterSetNode()
    volumeID = paramsNode.GetMasterVolumeNode().GetID()
    segmentationID = paramsNode.GetSegmentationNode().GetID()
    selectedSegmentID = paramsNode.GetSelectedSegmentID()
    segment = paramsNode.GetSegmentationNode().GetSegmentation().GetSegment(selectedSegmentID)
    priorSegmentationExists = segment.HasTag('PETTumorSegmentation.SegmentationId')
    oldVolumeID = segmentationParameters.GetPETVolumeReference()
    oldSegmentationID = segmentationParameters.GetSegmentationReference()
    oldSelectedSegmentID = segmentationParameters.GetSelectedSegmentID()

    if (volumeID!=oldVolumeID or segmentationID!=oldSegmentationID):
      self.reset()
    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    if not segmentationParameters: return
    if not self.vtkSegmentationLogic: self.vtkSegmentationLogic = slicer.vtkSlicerPETTumorSegmentationLogic()

    self.saveStateForUndo()
    self.updateMRMLFromGUI()

    #Get the fiducial nodes
    centerPointFiducialsNode = self.scene.GetNodeByID( str( segmentationParameters.GetCenterPointIndicatorListReference() ) )
    centerPointFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( centerPointFiducialsNode )
    globalRefinementFiducialNode = self.scene.GetNodeByID( str( segmentationParameters.GetGlobalRefinementIndicatorListReference() ) )
    globalRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( globalRefinementFiducialNode )
    localRefinementFiducialNode = self.scene.GetNodeByID( str( segmentationParameters.GetLocalRefinementIndicatorListReference() ) )
    localRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( localRefinementFiducialNode )

    #Set the current segment information and volumes
    segmentationParameters.SetPETVolumeReference(volumeID)
    segmentationParameters.SetSegmentationReference(segmentationID)
    segmentationParameters.SetSelectedSegmentID(selectedSegmentID)

    #If there are no center points, then the click should make a new center point and segmentation.
    #If the label has changed, go with this and remove center points to start a new segmentation.
    #If no refinement mode is on, remove points and start a new segmentation.
    if (centerPointFiducials.GetNumberOfControlPoints()==0 or selectedSegmentID!=oldSelectedSegmentID or segmentationParameters.GetNoRefinementOn()): # initialize segmentation
      # clear old data
      segmentationParameters.ClearInitialLabelMap()
      if (centerPointFiducials.GetNumberOfControlPoints()!=0):
        globalRefinementFiducial.RemoveAllControlPoints()
        localRefinementFiducial.RemoveAllControlPoints()
        centerPointFiducials.RemoveAllControlPoints()
      #add the new center point
      centerPointFiducials.AddControlPoint( vtk.vtkVector3d(rasCoorinate) )
      self.vtkSegmentationLogic.Apply( segmentationParameters, None )
    else:
      #If global refinement is active, then the existing segmentation should be refined globally.
      if (segmentationParameters.GetGlobalRefinementOn()): # perform global refinement, unless new label
        #Only one global refinement point at a time
        globalRefinementFiducial.RemoveAllControlPoints()
        globalRefinementFiducial.AddControlPoint( vtk.vtkVector3d(rasCoorinate) )
        self.vtkSegmentationLogic.ApplyGlobalRefinement( segmentationParameters, None )
      #If local refinement is active, then the existing segmentation should be refined localy.
      elif (segmentationParameters.GetLocalRefinementOn()): # perform local refinement, unless new label
        localRefinementFiducial.AddControlPoint( vtk.vtkVector3d(rasCoorinate) )
        self.vtkSegmentationLogic.ApplyLocalRefinement( segmentationParameters, None )

    # run segmentation algorithm
    self._showDebugInfo(segmentationParameters)
    #self.vtkSegmentationLogic.Apply( segmentationParameters, None )
    self.updateCurrentSegmentationID()

  def onApplyParameters(self):
    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    if not segmentationParameters: return

    paramsNode = self.scriptedEffect.parameterSetNode()
    volumeID = paramsNode.GetMasterVolumeNode().GetID()
    segmentationID = paramsNode.GetSegmentationNode().GetID()
    selectedSegmentID = paramsNode.GetSelectedSegmentID()
    segment = paramsNode.GetSegmentationNode().GetSegmentation().GetSegment(selectedSegmentID)
    priorSegmentationExists = segment.HasTag('PETTumorSegmentation.SegmentationId')
    oldVolumeID = segmentationParameters.GetPETVolumeReference()
    oldSegmentationID = segmentationParameters.GetSegmentationReference()
    oldSelectedSegmentID = segmentationParameters.GetSelectedSegmentID()

    if (volumeID!=oldVolumeID or segmentationID!=oldSegmentationID):
      self.reset()
    segmentationParameters = self.getPETTumorSegmentationParameterNode()
    if not segmentationParameters: return
    if not self.vtkSegmentationLogic: self.vtkSegmentationLogic = slicer.vtkSlicerPETTumorSegmentationLogic()

    centerPointFiducialsNode = self.scene.GetNodeByID( str( segmentationParameters.GetCenterPointIndicatorListReference() ) )
    centerPointFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( centerPointFiducialsNode )
    if (centerPointFiducials.GetNumberOfControlPoints()==0 or selectedSegmentID!=oldSelectedSegmentID):
      # there is no segmentation which could be changed by updated paramters
      return

    self.saveStateForUndo()
    # run segmentation algorithm with changed parameters
    self.updateMRMLFromGUI()
    self._showDebugInfo(segmentationParameters)
    self.vtkSegmentationLogic.Apply( segmentationParameters, None )
    self.updateCurrentSegmentationID()

  def updateCurrentSegmentationID(self):
    paramsNode = self.scriptedEffect.parameterSetNode()
    segmentation = paramsNode.GetSegmentationNode().GetSegmentation()
    segmentIDs = vtk.vtkStringArray()
    segmentation.GetSegmentIDs(segmentIDs)
    for index in range(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      segment = segmentation.GetSegment(segmentID)
      petSegmentationId = vtk.mutable("")
      if segment.GetTag('PETTumorSegmentation.SegmentationId',petSegmentationId):
        if int(str(petSegmentationId))>self.segmentationIdCounter:
          segment.RemoveTag('PETTumorSegmentation.SegmentationId')

    selectedSegmentID = paramsNode.GetSelectedSegmentID()
    segment = paramsNode.GetSegmentationNode().GetSegmentation().GetSegment(selectedSegmentID)
    self.segmentationIdCounter = self.segmentationIdCounter+1
    segment.SetTag('PETTumorSegmentation.SegmentationId', str(self.segmentationIdCounter))

  def getCurrentSegmentationID(self):
    paramsNode = self.scriptedEffect.parameterSetNode()
    segmentation = paramsNode.GetSegmentationNode().GetSegmentation()
    segmentIDs = vtk.vtkStringArray()
    segmentation.GetSegmentIDs(segmentIDs)
    currentSegmentationID = -1
    for index in range(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      segment = segmentation.GetSegment(segmentID)
      petSegmentationId = vtk.mutable("")
      if segment.GetTag('PETTumorSegmentation.SegmentationId',petSegmentationId):
        if int(str(petSegmentationId))>currentSegmentationID:
          currentSegmentationID = int(str(petSegmentationId))
    return currentSegmentationID

  # Safely return parameter node at right stage (considering potentialundos/redos)
  def getPETTumorSegmentationParameterNode(self):
    # search for segmentation parameter node corresponding to current state of segmentation considering potential undos/redos
    currentSegmentationId = self.getCurrentSegmentationID()
    if currentSegmentationId!=-1:
      if currentSegmentationId<self.segmentationIdCounter:
        numUndos = self.segmentationIdCounter-currentSegmentationId
        for i in range(numUndos):
          self._showDebugInfo(self._findPETTumorSegmentationParameterNodeInScene())
          self.scene.Undo()
          self._showDebugInfo(self._findPETTumorSegmentationParameterNodeInScene())
        self.segmentationIdCounter = currentSegmentationId
      if currentSegmentationId>self.segmentationIdCounter:
        numRedos = self.segmentationIdCounter-currentSegmentationId
        for i in range(numRedos):
          self.scene.Redo()
        self.segmentationIdCounter = currentSegmentationId
    else:
      # no prior segmentation state found -> clear state
      node = self._findPETTumorSegmentationParameterNodeInScene()
      if node:
        node.ClearInitialLabelMap()
        centerPointFiducialsNode = self.scene.GetNodeByID( str( node.GetCenterPointIndicatorListReference() ) )
        centerPointFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( centerPointFiducialsNode )
        globalRefinementFiducialNode = self.scene.GetNodeByID( str( node.GetGlobalRefinementIndicatorListReference() ) )
        globalRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( globalRefinementFiducialNode )
        localRefinementFiducialNode = self.scene.GetNodeByID( str( node.GetLocalRefinementIndicatorListReference() ) )
        localRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( localRefinementFiducialNode )
        centerPointFiducials.RemoveAllControlPoints()
        globalRefinementFiducial.RemoveAllControlPoints()
        localRefinementFiducial.RemoveAllControlPoints()
        self.segmentationIdCounter = 0

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

    centerFiducialList = slicer.vtkMRMLMarkupsFiducialNode() 
    centerFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( centerFiducialList )
    node.SetCenterPointIndicatorListReference( centerFiducialList.GetID() )

    globalRefinementFiducialList = slicer.vtkMRMLMarkupsFiducialNode() 
    globalRefinementFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( globalRefinementFiducialList )
    node.SetGlobalRefinementIndicatorListReference( globalRefinementFiducialList.GetID() )

    localRefinementFiducialList = slicer.vtkMRMLMarkupsFiducialNode() 
    localRefinementFiducialList.SetUndoEnabled(True)
    self.scene.AddNode( localRefinementFiducialList )
    node.SetLocalRefinementIndicatorListReference( localRefinementFiducialList.GetID() )

    return node

  def _showDebugInfo(self, node):
    if not self.debug: return

    #Get the fiducial nodes
    centerPointFiducialsNode = self.scene.GetNodeByID( str( node.GetCenterPointIndicatorListReference() ) )
    centerPointFiducials = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( centerPointFiducialsNode )
    globalRefinementFiducialNode = self.scene.GetNodeByID( str( node.GetGlobalRefinementIndicatorListReference() ) )
    globalRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( globalRefinementFiducialNode )
    localRefinementFiducialNode = self.scene.GetNodeByID( str( node.GetLocalRefinementIndicatorListReference() ) )
    localRefinementFiducial = slicer.vtkMRMLMarkupsFiducialNode.SafeDownCast( localRefinementFiducialNode )

    print('  volumeId='+str(node.GetPETVolumeReference()))
    print('  segmentationID='+str(node.GetSegmentationReference()))
    print('  selectedSegmentID='+str(node.GetSelectedSegmentID()))
    print('  #centerPointFiducials='+str(centerPointFiducials.GetNumberOfControlPoints()))
    print('  #globalRefinementFiducial='+str(globalRefinementFiducial.GetNumberOfControlPoints()))
    print('  #localRefinementFiducial='+str(localRefinementFiducial.GetNumberOfControlPoints()))

class SegmentEditorPETTumor(ScriptedLoadableModule):
  """
  This class is the 'hook' for slicer to detect and recognize the extension
  as a loadable scripted module
  """
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Segment Editor PET Tumor Segmentation"
    self.parent.categories = ["Developer Tools.Segment Editor Extensions"]
    self.parent.dependencies = ['Terminologies']
    self.parent.contributors = ["Christian Bauer (University of Iowa), Markus van Tol (University of Iowa), "
                                "Steve Pieper (Isomics)"] # insert your name in the list
    self.parent.hidden = True

    self.parent.helpText = """
    PET Tumor Segmentation Module
    """
    self.parent.acknowledgementText = """
    This editor extension was developed by
    Christian Bauer, University of Iowa and
    Markus van Tol, Univesity of Iowa
    based on work by:
    Steve Pieper, Isomics, Inc.
    based on work by:
    Jean-Christophe Fillion-Robin, Kitware Inc.
    and was partially funded by NIH grants U01CA140206, U24CA180918 and 3P41RR013218.
    """
    slicer.app.connect("startupCompleted()", self.registerEditorEffect)
    slicer.app.connect("startupCompleted()", self.loadTerminologyAndAnatomicContext)

  def registerEditorEffect(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    scriptedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    scriptedEffect.setPythonSource(__file__.replace('\\','/'))
    scriptedEffect.self().register()

  def loadTerminologyAndAnatomicContext(self):
    terminologyFile = os.path.join(os.path.dirname(__file__), 'SegmentationCategoryTypeModifier-HeadAndNeckCancer.json')
    anatomicContextFile = os.path.join(os.path.dirname(__file__), 'AnatomicRegionAndModifier-DICOM-HeadAndNeckCancer.json')
    terminologyLogic = slicer.modules.terminologies.logic()
    terminologyLogic.LoadTerminologyFromFile(terminologyFile)
    terminologyLogic.LoadAnatomicContextFromFile(anatomicContextFile)
