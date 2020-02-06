project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  #../../../../Slicer/Modules/Scripted/EditorLib/Logic
  #${CMAKE_CURRENT_SOURCE_DIR}/../../../../Slicer/Modules/Scripted/EditorLib/Logic
  #${Slicer_SOURCE_DIR}/Modules/Scripted/EditorLib/Logic
  #${Slicer_binary_dir}/Slicer-build/Modules/Scripted/EditorLib/Logic/vtkImageStash.h
  #message(${Slicer_binary_dir}/Modules/Scripted/EditorLib/Logic/vtkImageStash.h)
  #${Slicer_source_dir}/Modules/Scripted/EditorLib/Logic/vtkImageStash.h
  #
  ${MRML_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${Slicer_ModuleMRML_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleLogic_INCLUDE_DIRS}
  ${Slicer_ModuleSegmentations_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}ParametersNode.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  vtkSlicerSegmentationsModuleLogic
  ${ITK_LIBRARIES}
  vtkSlicerAnnotationsModuleMRML
  #vtkSlicerEditorLibModuleLogic
  vtkSlicerSegmentationsModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
