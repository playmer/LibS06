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
	void SonicXNTexture::read(File *file) {
		SonicXNSection::read(file);

		size_t texture_table_info_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		file->SetAddress(texture_table_info_address);

		unsigned int texture_count = file->Read<u32>();
		size_t texture_table_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		file->AddLabel(
			"Texture Table", 
			texture_table_address, 
			texture_table_address + (file_mode == MODE_YNO ? texture_count*8 : texture_count*20 + 4));

		for (size_t i=0; i<texture_count; i++) {
			if (file_mode == MODE_YNO) {
				file->SetAddress(texture_table_address + i*8);
			}
			else file->SetAddress(texture_table_address + i*20 + 4);

			size_t name_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			unsigned int name_size = file->Read<u32>();

			file->SetAddress(name_address);
			string texture_unit = file->ReadNullTerminatedString();

			textures.push_back(texture_unit);
			sizes.push_back(name_size);

			Error::printfMessage(Error::LogType::LOG, "Found texture unit %d: %s (Flags: %d)", i, texture_unit.c_str(), name_size);
		}
	}

	void SonicXNTexture::writeBody(File *file) {
		file->FixPadding(16);

		size_t texture_address=file->GetCurrentAddress();
		file->WriteByte(0, textures.size() * 20);

		size_t table_address=file->GetCurrentAddress();
		unsigned int texture_count = textures.size();
		file->Write<u32>(texture_count);
		file->WriteAddressFileEndianess(texture_address);

		texture_addresses.clear();
		for (size_t i=0; i<texture_count; i++) {
			texture_addresses.push_back(file->GetCurrentAddress());
			file->WriteNullTerminatedString(textures[i].c_str());
		}

		size_t bookmark=file->GetCurrentAddress();

		file->SetAddress(head_address + 8);
		file->Write<u32>(table_address); // TODO Table stuff: false

		for (size_t i=0; i<texture_count; i++) {
			file->SetAddress(texture_address + i*20 + 4);
			file->WriteAddressFileEndianess(texture_addresses[i]);
			file->Write<u32>(sizes[i]);
		}

		file->SetAddress(bookmark);
	}

	unsigned int SonicXNTexture::addTexture(std::string name) {
		for (size_t i=0; i<textures.size(); i++) {
			if (textures[i] == name) {
				return i;
			}
		}

		textures.push_back(name);
		unsigned int size=0x010004;
		sizes.push_back(size);
		return textures.size()-1;
	}
}