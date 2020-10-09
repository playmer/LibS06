//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
//
//    This file is part of SonicGLvl, a community-created free level editor 
//    for the PC version of Sonic Generations.
//
//    SonicGLvl is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SonicGLvl is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    
//
//    Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.
//=========================================================================

#include "S06Common.h"
#include "S06Collision.h"
#include "S06XnFile.h"
#include "File.hpp"
//#include "Havok.h"

namespace LibS06 {
	SonicCollision::SonicCollision(std::string filename) {
		File file(filename, File::Style::Read);

		if (file.Valid()) {
			read(&file);
			file.Close();
		}
	}

	void SonicCollisionFace::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_COLLISION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		v1 = file->Read<u16>(Endianess::Big);
		v2 = file->Read<u16>(Endianess::Big);
		v3 = file->Read<u16>(Endianess::Big);
		file->OffsetAddress(2);

		collision_flag = file->Read<u32>(Endianess::Big);
	}

	void SonicCollisionFace::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_COLLISION_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		file->Write<u16>(v1, Endianess::Big);
		file->Write<u16>(v2, Endianess::Big);
		file->Write<u16>(v3, Endianess::Big);
		file->WriteByte(0, 2);
		file->Write<u32>(collision_flag, Endianess::Big);
	}



	void SonicCollision::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_COLLISION_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		size_t address=0;

		file->SetRootNodeAddress(32);
		unsigned int file_size = file->Read<u32>(Endianess::Big);

		file->SetAddress(32);

		size_t geometry_address  = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		size_t mopp_code_address  = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);

		// Geometry
		file->SetAddress(geometry_address);
		size_t vertex_section_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		size_t face_section_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);

		// Vertices
		file->SetAddress(vertex_section_address);
		unsigned int vertex_total = file->Read<u32>(Endianess::Big);

		for (size_t i=0; i<vertex_total; i++) {
			glm::vec3 position = file->Read<glm::vec3>();
			vertex_pool.push_back(position);
		}

		// Faces
		file->SetAddress(face_section_address);

		unsigned int face_total = file->Read<u32>(Endianess::Big);

		for (size_t i=0; i<face_total; i++) {
			SonicCollisionFace face;
			face.read(file);
			face_pool.push_back(face);
		}

		// Mopp Code
		file->SetAddress(mopp_code_address);
		mopp_code_center = file->Read<glm::vec3>();
		mopp_code_w = file->Read<f32>(Endianess::Big);
		size_t mopp_code_size = file->Read<u32>(Endianess::Big);
		mopp_code_data.resize(mopp_code_size);
		for (auto& mode_code_data_element : mopp_code_data)
			mode_code_data_element = file->Read<u8>();
	}


	void SonicCollision::save(std::string filename) {
		File file(filename, File::Style::Write);

		if (file.Valid()) {
			write(&file);
			file.Close();
		}
	}

	void SonicCollision::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_COLLISION_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->SetRootNodeAddress(32);

		unsigned int file_size=0;
		unsigned int offset_table_address=0;

		file->WriteByte(0, 22);
		std::string header="1BBINA";
		file->WriteNullTerminatedString(header.c_str());
		file->FixPadding(32);

		SonicOffsetTable offset_table;

		size_t geometry_address=40;
		size_t unknown_address=0;
		size_t vertex_section_address=0;
		size_t face_section_address=0;

		file->WriteByte(0, 16);

		// Geometry
		// Vertices
		vertex_section_address = file->GetCurrentAddress();

		unsigned int vertex_total=vertex_pool.size();
		file->Write<u32>(vertex_total);

		for (size_t i=0; i<vertex_total; i++) {
			file->Write<glm::vec3>(vertex_pool[i]);
		}

		// Faces
		face_section_address = file->GetCurrentAddress();

		unsigned int face_total=face_pool.size();
		file->Write<u32>(face_total, Endianess::Big);
		for (size_t i=0; i<face_total; i++) {
			face_pool[i].write(file);
		}

		// Unknown
		unknown_address = file->GetCurrentAddress();

		file->Write<glm::vec3>(mopp_code_center);
		file->Write<f32>(mopp_code_w, Endianess::Big);
		file->Write<u32>(mopp_code_size, Endianess::Big);
		
		for (auto& mode_code_data_element : mopp_code_data)
			file->Write<u8>(mode_code_data_element);

		file->FixPadding(4);


		file->SetAddress(32);
		offset_table.addEntry(0x40, file->GetCurrentAddress());
		file->WriteAddress(geometry_address, Endianess::Big);
		offset_table.addEntry(0x41, file->GetCurrentAddress());
		file->WriteAddress(unknown_address, Endianess::Big);
		offset_table.addEntry(0x41, file->GetCurrentAddress());
		file->WriteAddress(vertex_section_address, Endianess::Big);
		offset_table.addEntry(0x41, file->GetCurrentAddress());
		file->WriteAddress(face_section_address, Endianess::Big);
		file->GoToEnd();

		offset_table_address = static_cast<u32>(file->GetCurrentAddress() - 32);
		file->WriteAddressTableBBIN();
		
		file_size = file->GetFileSize();

		file->SetAddress(0);
		file->Write<u32>(file_size, Endianess::Big);
		file->Write<u32>(offset_table_address, Endianess::Big);
		size_t offset_table_size = file_size - offset_table_address - 32;
		file->Write<u32>(offset_table_size, Endianess::Big);
	}


//	SonicCollision::SonicCollision(FBX *fbx) {
//		mopp_code_data = NULL;
//		mopp_code_size = 0;
//
//		FbxScene *lScene = fbx->getScene();
//
//		if (lScene) {
//			printf("Scene for collision found!\n");
//
//			const int lNodeCount = lScene->GetSrcObjectCount<FbxNode>();
//			FbxStatus lStatus;
//
//			for (int lIndex=0; lIndex<lNodeCount; lIndex++) {
//				FbxNode *lNode = lScene->GetSrcObject<FbxNode>(lIndex);
//				printf("Collision Node found: %s\n", lNode->GetName());
//				addFbxNode(lNode);
//			}
//		}
//
//		buildMoppCode();
//	}
//
//
//	void SonicCollision::addFbxNode(FbxNode *lNode) {
//		if (!lNode) return;
//
//		FbxMesh *lMesh=lNode->GetMesh();
//
//		printf("Looking for mesh\n");
//
//		if (!lMesh) return;
//
//		printf("Mesh found!\n");
//
//		std::string node_name = ToString(lNode->GetName());
//		unsigned int collision_flag = 0;
//
//		size_t pos=node_name.find_last_of("@");
//
//		if (pos != std::string::npos) {
//			FromString<unsigned int>(collision_flag, node_name.substr(pos+1, node_name.size() - pos - 1), std::hex);
//		}
//
//		FbxAMatrix transform_matrix=lNode->EvaluateGlobalTransform();
//
//		int lPolygonCount=lMesh->GetPolygonCount();
//		int control_points_count=lMesh->GetControlPointsCount();
//		FbxVector4 *control_points=lMesh->GetControlPoints();
//
//		for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
//			int polygon_size=lMesh->GetPolygonSize(lPolygonIndex);
//			if (polygon_size == 3) {
//				SonicCollisionFace face;
//				face.v1 = face.v2 = face.v3 = 0;
//				face.collision_flag = collision_flag;
//
//				for (int j=0; j<polygon_size; j++) {
//					int control_point_index=lMesh->GetPolygonVertex(lPolygonIndex, j);
//					FbxVector4 control_point=transform_matrix.MultT(control_points[control_point_index]);
//
//					glm::vec3 vertex_position = glm::vec3(control_point[0], control_point[2], -control_point[1]);
//					size_t vertex_index = 0;
//					bool found = false;
//					for (size_t i=0; i<vertex_pool.size(); i++) {
//						if (vertex_position == vertex_pool[i]) {
//							found = true;
//							vertex_index = i;
//							break;
//						}
//					}
//
//					if (!found) {
//						vertex_index = vertex_pool.size();
//						vertex_pool.push_back(vertex_position);
//					}
//
//					if (j == 0) face.v1=vertex_index;
//					if (j == 1) face.v2=vertex_index;
//					if (j == 2) face.v3=vertex_index;
//				}
//
//				face_pool.push_back(face);
//			}
//			else printf("Unsupported polygon size.\n");
//		}
//	}
//
//	void SonicCollision::buildMoppCode() {
//		hkpSimpleMeshShape *m_storageMeshShape = new hkpSimpleMeshShape(0.01f);
//
//		m_storageMeshShape->m_vertices.setSize(vertex_pool.size());
//		for(size_t ti = 0; ti < vertex_pool.size(); ti++) {
//			m_storageMeshShape->m_vertices[ti] = hkVector4(vertex_pool[ti].x/100.0f, vertex_pool[ti].y/100.0f, vertex_pool[ti].z/100.0f);
//		}
//
//		m_storageMeshShape->m_triangles.setSize(face_pool.size());
//		for(size_t ti = 0; ti < face_pool.size(); ti++) {
//			m_storageMeshShape->m_triangles[ti].m_a = face_pool[ti].v1;
//			m_storageMeshShape->m_triangles[ti].m_b = face_pool[ti].v2;
//			m_storageMeshShape->m_triangles[ti].m_c = face_pool[ti].v3;
//		}
//
//
//		hkpStorageExtendedMeshShape* extendedMesh = new hkpStorageExtendedMeshShape();
//		hkpExtendedMeshShape::TrianglesSubpart part;
//		part.m_numTriangleShapes = m_storageMeshShape->m_triangles.getSize();
//		part.m_numVertices = m_storageMeshShape->m_vertices.getSize();
//		part.m_vertexBase = (float*)m_storageMeshShape->m_vertices.begin();
//		part.m_stridingType = hkpExtendedMeshShape::INDICES_INT32;
//		part.m_vertexStriding = sizeof(hkVector4);
//		part.m_indexBase = m_storageMeshShape->m_triangles.begin();
//		part.m_indexStriding = sizeof(hkpSimpleMeshShape::Triangle);
//		extendedMesh->addTrianglesSubpart( part );
//
//		hkpMoppCompilerInput compiler_input;
//
//		hkpMoppCode* moppCode = hkpMoppUtility::buildCode(extendedMesh, compiler_input);
//		if (moppCode) {
//			mopp_code_center.x = moppCode->m_info.m_offset(0);
//			mopp_code_center.y = moppCode->m_info.m_offset(1);
//			mopp_code_center.z = moppCode->m_info.m_offset(2);
//			mopp_code_w        = moppCode->m_info.m_offset(3);
//			mopp_code_size     = moppCode->m_data.getSize();
//			mopp_code_data     = (unsigned char *) malloc(mopp_code_size);
//
//			for (size_t i=0; i<mopp_code_size; i++) {
//				mopp_code_data[i] = moppCode->m_data[i];
//			}
//		}
//	}
}