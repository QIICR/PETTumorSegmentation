cmake_minimum_required(VERSION 2.8.9)

project(PETTumorSegmentation)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.4/Extensions/PETTumorSegmentation")
set(EXTENSION_CATEGORY "Segmentation")
set(EXTENSION_CONTRIBUTORS "Christian Bauer (University of Iowa), Markus van Tol (University of Iowa), Andrey Fedorov (SPL), Ethan Ulrich (University of Iowa), Reinhard Beichel (University of Iowa), John Buatti (University of Iowa)")
set(EXTENSION_DESCRIPTION "Tumor and lymph node segmentation in PET scans")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/QIICR/PETTumorSegmentation/master/PETTumorSegmentation.png")
set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/0/04/PETTumorSegmentation_Effect_with_models.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(PETTumorSegmentation)
add_subdirectory(PETTumorSegmentationEffect)
add_subdirectory(SegmentEditorPETTumorSegmentationEffect)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
