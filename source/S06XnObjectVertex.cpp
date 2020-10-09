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
	void SonicVertex::read(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode) {
		size_t address=0;
		normal = glm::vec3(0,0,0);
		bone_indices[0]=bone_indices[1]=bone_indices[2]=bone_indices[3]=0;
		bone_weights_f[0]=1.0f;
		bone_weights_f[1]=bone_weights_f[2]=bone_weights_f[3]=0;

		float total_weight=0.0f;
		if (file_mode != MODE_ENO) {
			// Position
			if (vertex_flag & 0x1) {
				position = file->Read<glm::vec3>();
			}

			// Bone Weights
			if (vertex_flag & 0x7000) {
				bone_weights_f[0] = file->Read<float>();
				bone_weights_f[1] = file->Read<float>();
				bone_weights_f[2] = file->Read<float>();;
			}

			// Bone Indices
			if (vertex_flag & 0x400) {
				bone_indices[0] = file->Read<u8>();
				bone_indices[1] = file->Read<u8>();
				bone_indices[2] = file->Read<u8>();
				bone_indices[3] = file->Read<u8>();
			}
			else {
				total_weight += bone_weights_f[0] + bone_weights_f[1] + bone_weights_f[2];

				bone_indices[0] = 0;
				bone_indices[1] = ((bone_weights_f[1] > 0.0f) ? 1 : 0);
				bone_indices[2] = ((bone_weights_f[2] > 0.0f) ? 2 : 0);
				bone_indices[3] = ((total_weight < 0.999f)    ? 3 : 0);
			}

			// Normal
			if (vertex_flag & 0x2) {
				normal = file->Read<glm::vec3>();
			}

			// RGBA 1
			if (vertex_flag & 0x8) {
				rgba[2] = file->Read<u8>();
				rgba[1] = file->Read<u8>();
				rgba[0] = file->Read<u8>();
				rgba[3] = file->Read<u8>();
			}

			// RGBA 2
			if (vertex_flag & 0x10) {
				rgba_2[2] = file->Read<u8>();
				rgba_2[1] = file->Read<u8>();
				rgba_2[0] = file->Read<u8>();
				rgba_2[3] = file->Read<u8>();
			}

			// UV Channel 1
			size_t uv_channels = vertex_flag / (0x10000);
			for (size_t i=0; i<uv_channels; i++) {
				uv[i] = file->Read<glm::vec2>();
			}

			// Tangent / Binormal
			if (vertex_flag & 0x140) {
				tangent = file->Read<glm::vec3>(); 
				binormal = file->Read<glm::vec3>();
			}
			
			bone_weights_f[3] = 1.0 - bone_weights_f[0] - bone_weights_f[1] - bone_weights_f[2];
			if (bone_weights_f[3] == 1.0) bone_weights_f[3] = 0.0;
		}
		else {
			if (vertex_flag == 0x310005) {
				position = file->Read<glm::vec3>();
				normal = file->ReadNormal360();
				uv[0].x = file->ReadHalf();
				uv[0].y = file->ReadHalf();
			}
			else if (vertex_flag == 0x317405) {
				position = file->Read<glm::vec3>();
				normal = file->Read<glm::vec3>();
				file->OffsetAddress(8);
				uv[0].x = file->ReadHalf();
				uv[0].y = file->ReadHalf();
			}
			else if (vertex_flag == 0x317685) {
				position = file->Read<glm::vec3>();
				normal = file->Read<glm::vec3>();
				file->OffsetAddress(8);
				uv[0].x = file->ReadHalf();
				uv[0].y = file->ReadHalf();
				file->OffsetAddress(8);
			}
			else {
				printf("Unhandled vertex flag: %d\n", vertex_flag);
				getchar();
			}
		}
	}

	void SonicVertex::write(File *file, unsigned int vertex_size, bool big_endian, unsigned int vertex_flag, XNFileMode file_mode) {
		// Position
		if (vertex_flag & 0x1) {
			file->Write<glm::vec3>(position);
		}

		// Bone Weights
		if (vertex_flag & 0x7000) {
			file->Write<float>(bone_weights_f[0]);
			file->Write<float>(bone_weights_f[1]);
			file->Write<float>(bone_weights_f[2]);
		}

		// Bone Indices
		if (vertex_flag & 0x400) {
			file->Write<u8>(bone_indices[0]);
			file->Write<u8>(bone_indices[1]);
			file->Write<u8>(bone_indices[2]);
			file->Write<u8>(bone_indices[3]);
		}

		// Normal
		if (vertex_flag & 0x2) {
			file->Write<glm::vec3>(normal);
		}

		// RGBA 1, stored in the file as BGRA
		if (vertex_flag & 0x8) {
			file->Write<u8>(rgba[2]);
			file->Write<u8>(rgba[1]);
			file->Write<u8>(rgba[0]);
			file->Write<u8>(rgba[3]);
		}

		// RGBA 2, stored in the file as BGRA
		if (vertex_flag & 0x10) {
			file->Write<u8>(rgba_2[2]);
			file->Write<u8>(rgba_2[1]);
			file->Write<u8>(rgba_2[0]);
			file->Write<u8>(rgba_2[3]);
		}

		// UV Channel 1
		size_t uv_channels = vertex_flag / (0x10000);
		for (size_t i=0; i<uv_channels; i++) {
			file->Write<glm::vec2>(uv[i]);
		}

		// Tangent / Binormal
		if (vertex_flag & 0x140) {
			file->Write<glm::vec3>(tangent);
			file->Write<glm::vec3>(binormal);
		}
			
	}

	void SonicVertexTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		unsigned int table_count = file->Read<u32>();
		size_t table_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		if (table_count > 1) {
			printf("Unhandled Case Vertex Table %d.\n", table_count);
			getchar();
		}

		file->SetAddress(table_address);

		flag_1 = file->Read<u32>();
		flag_2 = file->Read<u32>();
		vertex_size = file->Read<u32>();
		unsigned int vertex_count = file->Read<u32>();
		size_t vertex_offset = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		unsigned int bone_table_count= file->Read<u32>();
		size_t bone_table_offset = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		printf("%d %d %d\n", vertex_size, (int)vertex_offset, bone_table_count);
		printf("Flags: %d %d\n", flag_1, flag_2);

		Error::AddMessage(Error::LogType::LOG, "Vertex Size / Vertex Flag 1 / Vertex Flag 2: " + ToString(vertex_size) + " / " + ToString(flag_1) + " / " + ToString(flag_2));

		if(bone_table_count > 32) {
			printf("Bone table is bigger than 32! Size: %d\n", bone_table_count);
			getchar();
		}

		std::string bone_table_str="Bone Blending Table: ";
		for (size_t i=0; i<bone_table_count; i++) {
			file->SetAddress(bone_table_offset + i*4);
			unsigned int bone = file->Read<u32>();
			bone_table.push_back(bone);

			bone_table_str += ToString(bone) + " ";
		}

		Error::AddMessage(Error::LogType::LOG, "Vertex Table with a bone blending table of size " + ToString(bone_table.size()));

		for (size_t i=0; i<vertex_count; i++) {
			file->SetAddress(vertex_offset + i * vertex_size);
			SonicVertex *vertex = new SonicVertex();
			vertex->read(file, vertex_size, big_endian, flag_1, file_mode);
			vertices.push_back(vertex);
		}

		printf("Done reading vertices...\n");
	}

	void SonicVertexTable::writeVertices(File *file, XNFileMode file_mode) {
		vertex_buffer_address = file->GetCurrentAddress();

		for (size_t i=0; i<vertices.size(); i++) {
			vertices[i]->write(file, vertex_size, false, flag_1, file_mode);
		}
	}

	void SonicVertexTable::writeTable(File *file) {
		size_t bone_table_address = file->GetCurrentAddress();
		for (size_t i=0; i<bone_table.size(); i++) {
			file->Write<u32>(bone_table[i]);
		}

		vertex_table_address = file->GetCurrentAddress();

		file->Write<u32>(flag_1);
		file->Write<u32>(flag_2);
		file->Write<u32>(vertex_size);
		unsigned int vertex_count = vertices.size();
		file->Write<u32>(vertex_count);
		file->WriteByte(0, 4);

		unsigned int bone_table_count = bone_table.size();
		file->Write<u32>(bone_table_count);
		if (bone_table_count) file->WriteAddressFileEndianess(bone_table_address);
		else file->WriteByte(0, 4);

		file->WriteByte(0, 20);
	}

	void SonicVertexTable::writeTableFixed(File *file) {
		file->SetAddress(vertex_table_address + 16);
		file->Write<u32>(vertex_buffer_address);
	}

	void SonicVertexTable::write(File *file) {
		unsigned int total=1;
		file->Write<u32>(total);
		file->Write<u32>(vertex_table_address);
	}

	void SonicVertexTable::setScale(float scale) {
		for (size_t i=0; i<vertices.size(); i++) {
			vertices[i]->setScale(scale);
		}
	}
};
