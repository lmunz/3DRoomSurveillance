# minimum required cmake version
cmake_minimum_required(VERSION 3.2)
# set project name, version and language mode for the compiler
# CXX means C++
project(PointCloudMatching VERSION 1.0 LANGUAGES CXX)


set("OpenCV_DIR" "C:/opencv/opencv/build/install")

# Makes sure that projects and sub projects are put into
# a nice tree structure inside visual studio
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# The options need to be the same as Open3D's default
# If Open3D is configured and built with custom options, you'll also need to
# specify the same custom options.
option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" ON)
if(STATIC_WINDOWS_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
 
# Dependencies
find_package(OpenCV REQUIRED)
find_package(Open3D REQUIRED)
 
# Application
add_executable(${CMAKE_PROJECT_NAME} "main.cpp")
# link OpenCV
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${OpenCV_LIBS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Open3D::Open3D)

# On Windows if BUILD_SHARED_LIBS is enabled, copy .dll files to the executable directory
if(WIN32)
    get_target_property(open3d_type Open3D::Open3D TYPE)
    if(open3d_type STREQUAL "SHARED_LIBRARY")
        message(STATUS "Copying Open3D.dll to ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
        add_custom_command(TARGET Draw POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy
                                   ${CMAKE_INSTALL_PREFIX}/bin/Open3D.dll
                                   ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
endif()
