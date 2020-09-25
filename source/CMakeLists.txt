cmake_minimum_required(VERSION 3.16 FATAL_ERROR) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  
project(libS06 CXX C)

add_executable(libS06)

target_sources(libS06
    PRIVATE
        File.hpp
        main.cpp
        S06Collision.cpp
        S06Collision.h
        S06Common.cpp
        S06Common.h
        S06DAE.cpp
        S06Set.cpp
        S06Set.h
        S06Text.cpp
        S06Text.h
        S06XnBones.cpp
        S06XnEffect.cpp
        S06XnFile.cpp
        S06XnFile.h
        S06XnFileFBX.cpp
        S06XnMotion.cpp
        S06XnObject.cpp
        S06XnObjectBone.cpp
        S06XnObjectIndex.cpp
        S06XnObjectMaterial.cpp
        S06XnObjectMesh.cpp
        S06XnObjectOldMaterial.cpp
        S06XnObjectPolygon.cpp
        S06XnObjectVertex.cpp
        S06XnObjectVertexResource.cpp
        S06XnTexture.cpp
)

target_include_directories(libS06 
    PUBLIC 
        ../dependencies/half
        ../dependencies/sha1
        ../dependencies/tinyxml
        ../dependencies/tristripper
)
