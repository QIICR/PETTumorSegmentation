cmake_minimum_required(VERSION 2.8.6)

#-----------------------------------------------------------------------------
set(MODULE_NAME PETTumorSegmentationEffect)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  PETTumorSegmentationEffect.py
  )

set(MODULE_PYTHON_RESOURCES
  PETTumorSegmentationEffect.png
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildScriptedModule(
  NAME PETTumorSegmentationEffect
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  #WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

