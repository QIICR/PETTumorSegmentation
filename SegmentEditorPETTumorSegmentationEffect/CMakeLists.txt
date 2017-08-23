cmake_minimum_required(VERSION 2.8.6)

#-----------------------------------------------------------------------------
set(MODULE_NAME SegmentEditorPETTumorSegmentationEffect)
  
#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  SegmentEditorPETTumor.py
  )

set(MODULE_PYTHON_RESOURCES
  PETTumorEffect.png
  SegmentationCategoryTypeModifier-HeadAndNeckCancer.json
  AnatomicRegionAndModifier-DICOM-HeadAndNeckCancer.json
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildScriptedModule(
  NAME SegmentEditorPETTumorSegmentationEffect
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  WITH_GENERIC_TESTS
  )

