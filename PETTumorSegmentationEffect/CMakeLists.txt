cmake_minimum_required(VERSION 2.8.6)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME PETTumorSegmentationEffect)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Modules/PETTumorSegmentationEffect")
  set(EXTENSION_CATEGORY “Editor Effects”)
  set(EXTENSION_CONTRIBUTORS “Christian Bauer (University of Iowa), Markus van Tol (University of Iowa)“)
  set(EXTENSION_DESCRIPTION “Editor effect for segmentation of tumors and lymph nodes in PET scans“)
endif()

#-----------------------------------------------------------------------------
set(MODULE_NAME PETTumorSegmentationEffect)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  PETTumorSegmentationEffect.py
  )

set(MODULE_PYTHON_RESOURCES
  PETTumorSegmentationEffect.png
  )

#-----------------------------------------------------------------------------
ctkMacroCompilePythonScript(
  TARGET_NAME PETTumorSegmentationEffect
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_ROOT}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
