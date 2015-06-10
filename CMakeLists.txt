cmake_minimum_required(VERSION 2.8)
 
PROJECT(ImageSlice)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(ImageSlice MACOSX_BUNDLE ImageSlice.cxx)
 
if(VTK_LIBRARIES)
  target_link_libraries(ImageSlice ${VTK_LIBRARIES})
else()
  target_link_libraries(ImageSlice vtkHybrid vtkWidgets)
endif()
