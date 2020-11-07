cmake_minimum_required(VERSION 2.8)

# project name
set(PROJ_NAME gstdualfisheye)
project(${PROJ_NAME})

# load the module to use pkg-config
find_package(PkgConfig REQUIRED)

# search library with pkg-config for gstreamer-1.0 and gstreamer-video-1.0
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)

# search library for opencv
find_package(OpenCV)
#include_directories(${OPENCV_INCLUDE_DIRS})
#link_directories(${OPENCV_LIBRARY_DIRS})

# target
add_library(${PROJ_NAME} SHARED gstdualfisheye.cpp)

# include directories
set(INCLUDE_DIRS 
    ${GSTREAMER_INCLUDE_DIRS} 
    ${GSTREAMER_VIDEO_INCLUDE_DIRS} 
    ${OPENCV_INCLUDE_DIRS}
)
include_directories("${INCLUDE_DIRS}")

# link library
set(LIBRARIES 
    ${GSTREAMER_LIBRARIES} 
    ${GSTREAMER_VIDEO_LIBRARIES} 
    ${OPENCV_LIBRARIES}
)
target_link_libraries(${PROJ_NAME} ${LIBRARIES})
