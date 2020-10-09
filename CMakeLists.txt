cmake_minimum_required(VERSION 3.16 FATAL_ERROR) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  
project(libS06 CXX C)

set(RepoDirectory ${CMAKE_CURRENT_LIST_DIR})
set(DependenciesDirectory ${RepoDirectory}/Dependencies)
set(AssetDirectory ${RepoDirectory}/Assets)

add_subdirectory(dependencies)
add_subdirectory(source)

add_executable(LibS06_TestProgram main.cpp)

target_include_directories(LibS06_TestProgram 
PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    ${DependenciesDirectory}/glm)

target_link_libraries(LibS06_TestProgram 
PUBLIC 
    LibS06 
    SOIS
)

set_target_properties(LibS06_TestProgram PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${AssetDirectory}/Bin)