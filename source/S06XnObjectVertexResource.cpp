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
	void SonicVertexResourceTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		unsigned int table_count = file->Read<u32>();
		size_t table_address = file->ReadAddressFileEndianess();

		if ((table_count != 1) && (table_count != 16)) {
			printf("Unhandled Case Vertex Resource Table %d.\n", table_count);
			getchar();
		}

		Error::AddMessage(Error::LogType::LOG, "Vertex Resource Table of type " + ToString(table_count) + " at " + ToString(table_address));

		file->SetAddress(table_address);

		unsigned short position_type_flag = file->Read<u16>();
		unsigned short position_total = file->Read<u16>();
		size_t position_address = file->ReadAddressFileEndianess();

		unsigned short normal_type_flag = file->Read<u16>();
		unsigned short normal_total = file->Read<u16>();
		size_t normal_address = file->ReadAddressFileEndianess();

		unsigned short color_type_flag = file->Read<u32>();
		unsigned short color_total = file->Read<u32>();
		size_t color_address = file->ReadAddressFileEndianess();

		unsigned short uv_type_flag = file->Read<u32>();
		unsigned short uv_total = file->Read<u32>();
		size_t uv_address = file->ReadAddressFileEndianess();

		unsigned short uv2_type_flag = file->Read<u32>();
		unsigned short uv2_total = file->Read<u32>();
		size_t uv2_address = file->ReadAddressFileEndianess();

		unsigned short bones_type_flag = file->Read<u32>();
		unsigned short bones_total = file->Read<u32>();
		size_t bones_address = file->ReadAddressFileEndianess();

		unsigned short unknown_type_flag = file->Read<u32>();
		unsigned short unknown_total = file->Read<u32>();
		size_t unknown_address = file->ReadAddressFileEndianess();

		Error::AddMessage(Error::LogType::LOG, "  Positions: " + ToString(position_total) + "(" + ToString(position_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  Normals  : " + ToString(normal_total)   + "(" + ToString(normal_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  Colors   : " + ToString(color_total) + "(" + ToString(color_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  UVs      : " + ToString(uv_total) + "(" + ToString(uv_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  UV2s     : " + ToString(uv2_total) + "(" + ToString(uv2_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  Bones    : " + ToString(bones_total) + "(" + ToString(bones_type_flag) + ")");
		Error::AddMessage(Error::LogType::LOG, "  Unknown  : " + ToString(unknown_total) + "(" + ToString(unknown_type_flag) + ")");

		if (unknown_total > 0) {
			printf("The unknown total in the vertex resource table is used, not cracked yet.\n");
			getchar();
		}

		for (size_t i=0; i<position_total; i++) {
			float fx = 0;
			float fy = 0;
			float fz = 0;

			if (position_type_flag == 1) {
				file->SetAddress(position_address + i*12);
				fx = file->Read<f32>();
				fy = file->Read<f32>();
				fz = file->Read<f32>();
			}
			else if ((position_type_flag >= 3) && (position_type_flag <= 8)) {
				file->SetAddress(position_address + i*6);

				unsigned short pow_factor=position_type_flag-2;
				float div_factor=pow(4.0, (double)pow_factor);

				short f=0;
				f = (short)file->Read<u16>();
				fx = f/div_factor;
				f = (short)file->Read<u16>();
				fy = f/div_factor;
				f = (short)file->Read<u16>();
				fz = f/div_factor;
			}
			else {
				printf("Unhandled position type case %d at address %d\n", position_type_flag, (int)position_address);
				getchar();
			}

			glm::vec3 position(fx, fy, fz);
			positions.push_back(position);
		}


		
		for (size_t i=0; i<normal_total; i++) {
			float fx = 0.0;
			float fy = 0.0;
			float fz = 0.0;

			if (normal_type_flag == 3) {
				file->SetAddress(normal_address + i*3);

				char f=0;
				f= (char)file->Read<unsigned char>();
				fx = ((int)f);
				f= (char)file->Read<unsigned char>();
				fy = ((int)f);
				f= (char)file->Read<unsigned char>();
				fz = ((int)f);
			}
			else {
				printf("Unhandled normal type case %d at address %d\n", normal_type_flag, (int)normal_address);
				getchar();
			}

			glm::vec3 normal(fx, fy, fz);
			glm::normalize(normal);
			normals.push_back(normal);
		}


		
		for (size_t i=0; i<color_total; i++) {
			glm::vec4 color;

			if (color_type_flag == 1) {
				file->SetAddress(color_address + i*4);
				color = file->ReadRGBA8();
			}
			else {
				printf("Unhandled color type case %d at address %d\n", color_type_flag, (int)color_address);
				getchar();
			}

			colors.push_back(color);
		}


		for (size_t i=0; i<uv_total; i++) {
			float fx = 0.0;
			float fy = 0.0;

			if (uv_type_flag == 2) {
				file->SetAddress(uv_address + i*4);

				short f=0;
				f = (short)file->Read<u16>();
				fx = f/256.0;
				f = (short)file->Read<u16>();
				fy = f/256.0;
			}
			else if (uv_type_flag == 3) {
				file->SetAddress(uv_address + i*4);

				short f=0;
				f = (short)file->Read<u16>();
				fx = f/1024.0;
				f = (short)file->Read<u16>();
				fy = f/1024.0;
			}
			else {
				printf("Unhandled UV type case %d at address %d\n", uv_type_flag, (int)uv_address);
				getchar();
			}

			glm::vec2 uv(fx, fy);
			uvs.push_back(uv);
		}

		
		for (size_t i=0; i<uv2_total; i++) {
			float fx = 0.0;
			float fy = 0.0;

			if (uv2_type_flag == 2) {
				file->SetAddress(uv2_address + i*4);

				short f=0;
				f = (short)file->Read<u16>();
				fx = f/256.0;
				f  = (short)file->Read<u16>();
				fy = f/256.0;
			}
			else if (uv2_type_flag == 3) {
				file->SetAddress(uv2_address + i*4);

				short f=0;
				f = (short)file->Read<u16>();
				fx = f/1024.0;
				f = (short)file->Read<u16>();
				fy = f/1024.0;
			}
			else {
				printf("Unhandled UV2 type case %d at address %d\n", uv2_type_flag, (int)uv2_address);
				getchar();
			}

			glm::vec2 uv(fx, fy);
			uvs_2.push_back(uv);
		}

		
		for (size_t i=0; i<bones_total; i++) {
			SonicVertexBoneData bone_data;
			bone_data.weight = 0;

			if (bones_type_flag == 1) {
				file->SetAddress(bones_address + i*4);
				bone_data.bone_1 = file->Read<u8>();
				bone_data.bone_2 = file->Read<u8>();
				bone_data.weight = file->Read<u16>(Endianess::Big);
				bones.push_back(bone_data);
			}
			else {
				printf("Unhandled Bones type case %d at address %d\n", bones_type_flag, (int)bones_address);
				getchar();
			}
		}
	}
};
