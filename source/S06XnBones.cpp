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
	void SonicXNBones::read(File *file) {
		SonicXNSection::read(file);

		size_t table_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		file->SetAddress(table_address+4);

		unsigned int bones_count = file->Read<u32>();
		size_t bones_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		for (size_t i=0; i<bones_count; i++) {
			file->SetAddress(bones_address + i*8);

			unsigned int index = file->Read<u32>();
			size_t name_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

			file->SetAddress(name_address);
			std::string name = file->ReadNullTerminatedString();

			bone_indices.push_back(index);
			bone_names.push_back(name);
		}
	}

	
	void SonicXNBones::writeBody(File *file) {
		file->FixPadding(16);

		size_t bone_address=file->GetCurrentAddress();
		file->WriteByte(0, bone_names.size() * 8);

		size_t table_address=file->GetCurrentAddress();
		unsigned int bone_count = bone_names.size();

		file->WriteByte(0, 4);
		file->Write<u32>(bone_count);
		file->WriteAddressFileEndianess(bone_address);

		bone_names_addresses.clear();
		for (size_t i=0; i<bone_count; i++) {
			bone_names_addresses.push_back(file->GetCurrentAddress());
			file->WriteNullTerminatedString(bone_names[i].c_str());
		}

		size_t bookmark=file->GetCurrentAddress();

		file->SetAddress(head_address + 8);
		file->WriteAddressFileEndianess(table_address); // TODO tables: this one is false

		for (size_t i=0; i<bone_count; i++) {
			file->SetAddress(bone_address + i*8);

			file->Write<u32>(bone_indices[i]);
			file->WriteAddressFileEndianess(bone_names_addresses[i]);
		}

		file->SetAddress(bookmark);
	}
}