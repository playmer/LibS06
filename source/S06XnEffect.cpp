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

#include "S06XnFile.h"

namespace LibS06 {
	void SonicXNEffect::read(File *file) {
		SonicXNSection::read(file);

		size_t table_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		file->SetAddress(table_address+4);

		unsigned int shader_count = file->Read<u32>();
		size_t shader_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		unsigned int name_count = file->Read<u32>();
		size_t name_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		unsigned int extras_count = file->Read<u32>();
		size_t extras_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		for (size_t i=0; i<shader_count; i++) {
			file->SetAddress(shader_address + i*8 + 4);
			size_t string_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			file->SetAddress(string_address);
			std::string shader = file->ReadNullTerminatedString();
			material_shaders.push_back(shader);
		}


		for (size_t i=0; i<name_count; i++) {
			file->SetAddress(name_address + i*12 + 4);
			unsigned int index = file->Read<u32>();
			size_t string_address = file->ReadAddress(big_endian, __FILE__, __LINE__, __PRETTY_FUNCTION__);
			file->SetAddress(string_address);
			std::string name = file->ReadNullTerminatedString();
			material_indices.push_back(index);
			material_names.push_back(name);
		}

		for (size_t i=0; i<extras_count; i++) {
			file->SetAddress(extras_address + i*2);
			unsigned short extra=0;
			extra = file->Read<u16>();
			extras.push_back(extra);
		}
	}

	
	void SonicXNEffect::writeBody(File *file) {
		file->FixPadding(16);

		size_t shader_address=file->GetCurrentAddress();
		file->WriteByte(0, material_shaders.size() * 8);

		size_t name_address=file->GetCurrentAddress();
		file->WriteByte(0, material_names.size() * 12);

		size_t extras_address=file->GetCurrentAddress();
		file->WriteByte(0, extras.size() * 2);
		file->FixPadding(4);

		size_t table_address=file->GetCurrentAddress();
		unsigned int shader_count = material_shaders.size();
		unsigned int name_count = material_names.size();
		unsigned int extras_count = extras.size();

		file->WriteByte(0, 4);
		file->Write<u32>(shader_count);
		file->WriteAddressFileEndianess(shader_address);

		file->Write<u32>(name_count);
		file->WriteAddressFileEndianess(name_address);

		file->Write<u32>(extras_count);
		file->WriteAddressFileEndianess(extras_address);


		material_shaders_addresses.clear();
		for (size_t i=0; i<shader_count; i++) {
			material_shaders_addresses.push_back(file->GetCurrentAddress());
			file->WriteNullTerminatedString(material_shaders[i].c_str());
		}

		material_names_addresses.clear();
		for (size_t i=0; i<name_count; i++) {
			material_names_addresses.push_back(file->GetCurrentAddress());
			file->WriteNullTerminatedString(material_names[i].c_str());
		}

		size_t bookmark=file->GetCurrentAddress();
		
		file->SetAddress(head_address + 8);
		file->WriteAddressFileEndianess(table_address); // TODO: TABLE ADDRESSES this passed true

		for (size_t i=0; i<shader_count; i++) {
			file->SetAddress(shader_address + i*8 + 4);
			file->WriteAddressFileEndianess(material_shaders_addresses[i]);
		}

		for (size_t i=0; i<name_count; i++) {
			file->SetAddress(name_address + i*12 + 4);
			file->Write<u32>(material_indices[i]);
			file->WriteAddressFileEndianess(material_names_addresses[i]);
		}

		for (size_t i=0; i<extras_count; i++) {
			file->SetAddress(extras_address + i*2);
			file->Write(extras[i]);
		}


		file->SetAddress(bookmark);
	}
}
