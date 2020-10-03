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

#include <algorithm>
#include "S06XnFile.h"

namespace LibS06 {
	void SonicSubmesh::read(File *file, bool big_endian, XNFileMode file_mode) {
		center = file->Read<glm::vec3>();
		radius = file->Read<f32>();
		node_index = file->Read<u32>();
		matrix_index = file->Read<u32>();
		material_index = file->Read<u32>();
		vertex_index = file->Read<u32>();
		indices_index = file->Read<u32>();

		if (file_mode != MODE_GNO) {
			indices_index_2 = file->Read<u32>();

			if (indices_index != indices_index_2) {
				printf("Unhandled case! Submesh Index 1 and 2 are different! (%d vs %d)", indices_index, indices_index_2);
				getchar();
			}
		}

		printf("Found submesh with:\n Position: %f %f %f Radius: %f\n Node Index: %d Matrix Index: %d Material Index: %d Vertex Index: %d\n Indices Index: %d Indices Index 2: %d\n", center.x, center.y, center.z, radius, node_index, matrix_index, material_index, vertex_index, indices_index, indices_index_2);

		Error::AddMessage(Error::LogType::WARNING, "Submesh:");
		Error::AddMessage(Error::LogType::WARNING, "  Node Index:     " + ToString(node_index));
		Error::AddMessage(Error::LogType::WARNING, "  Matrix Index:   " + ToString(matrix_index));
		Error::AddMessage(Error::LogType::WARNING, "  Material Index: " + ToString(material_index));
		Error::AddMessage(Error::LogType::WARNING, "  Vertex Index:   " + ToString(vertex_index));
		Error::AddMessage(Error::LogType::WARNING, "  Indices Index:  " + ToString(indices_index));
	}

	void SonicSubmesh::write(File *file) {
		file->Write<glm::vec3>(center); // TODO: Tables: false
		file->Write<f32>(radius);
		file->Write<u32>(node_index);
		file->Write<u32>(matrix_index);
		file->Write<u32>(material_index);
		file->Write<u32>(vertex_index);
		file->Write<u32>(indices_index);
		file->Write<u32>(indices_index_2);
	}

	void SonicMesh::read(File *file, bool big_endian, XNFileMode file_mode) {
		flag = file->Read<u32>();
		unsigned int submesh_count = file->Read<u32>();
		size_t submesh_offset = file->ReadAddressFileEndianess();
		unsigned int extra_count = file->Read<u32>();
		size_t extra_offset = file->Read<u32>();

		Error::AddMessage(Error::LogType::WARNING, "Mesh (" + ToString(flag) + ") found with " + ToString(submesh_count) + " submeshes.");

		for (size_t i=0; i<submesh_count; i++) {
			if (file_mode != MODE_GNO) {
				file->SetAddress(submesh_offset + i * 40);
			}
			else {
				file->SetAddress(submesh_offset + i * 36);
			}

			SonicSubmesh *submesh = new SonicSubmesh();
			submesh->read(file, big_endian, file_mode);
			submeshes.push_back(submesh);
		}

		printf("Found %d extras: ", extra_count);

		for (size_t i=0; i<extra_count; i++) {
			file->SetAddress(extra_offset + i*4);

			unsigned int extra = file->Read<u32>();
			extras.push_back(extra);

			printf("%d ", extra);
		}
		printf("\n", flag, submesh_count);
	}

	void SonicMesh::writeSubmeshes(File *file) {
		submesh_table_address = file->GetCurrentAddress();
		for (size_t i=0; i<submeshes.size(); i++) {
			submeshes[i]->write(file);
		}
	}

	void SonicMesh::writeExtras(File *file) {
		extra_table_address = file->GetCurrentAddress();
		for (size_t i=0; i<extras.size(); i++) {
			file->Write<u32>(extras[i]);
		}
	}

	void SonicMesh::write(File *file) {
		file->Write<u32>(flag);
		unsigned int submeshes_count = submeshes.size();
		unsigned int extras_count = extras.size();
		file->Write<u32>(submeshes_count);
		file->WriteAddressFileEndianess(submesh_table_address);
		file->Write<u32>(extras_count);
		file->WriteAddressFileEndianess(extra_table_address);
	}
	
};
