cmake_minimum_required(VERSION 3.16 FATAL_ERROR) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  
project(libS06 CXX C)

add_subdirectory(dependencies)
add_subdirectory(source)

add_executable(test main.cpp)

target_link_libraries(test PUBLIC LibS06)
