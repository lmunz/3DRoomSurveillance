# minimum required cmake version
cmake_minimum_required(VERSION 3.2)
# set project name, version and language mode for the compiler
# CXX means C++
project(CVTestProject VERSION 1.0 LANGUAGES CXX)
 
# Makes sure that projects and sub projects are put into
# a nice tree structure inside visual studio
if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()
 
# Dependencies
find_package(OpenCV REQUIRED)
 
# Application
add_executable(${CMAKE_PROJECT_NAME} "main.cpp")
# link OpenCV
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${OpenCV_LIBS})