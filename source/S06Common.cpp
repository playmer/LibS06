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

#include <string>

#include "S06Common.h"
#include "S06XnFile.h"

#include "File.hpp"

namespace LibS06 {
	void SonicStringTable::writeString(File *file, std::string str) {
		file->WriteByte(0, 4);

		if (!str.size()) {
			SonicString new_string;
			new_string.addresses.clear();
			new_string.addresses.push_back(file->GetCurrentAddress()-4);
			new_string.value = "";
			strings.insert(strings.begin()+null_string_count, new_string);
			null_string_count++;
			return;
		}

		for (size_t i=0; i<strings.size(); i++) {
			if (strings[i].value == str) {
				strings[i].addresses.push_back(file->GetCurrentAddress()-4);
				return;
			}
		}

		SonicString new_string;
		new_string.addresses.clear();
		new_string.addresses.push_back(file->GetCurrentAddress()-4);
		new_string.value = str;
		strings.push_back(new_string);
	}

	void SonicStringTable::write(File *file) {
		for (size_t i=0; i<strings.size(); i++) {
			size_t address=file->GetCurrentAddress();
			if (strings[i].value.size()) file->WriteNullTerminatedString(strings[i].value.c_str());
			else {
				file->WriteByte(0, 4);
			}

			for (size_t j=0; j<strings[i].addresses.size(); j++) {
				file->SetAddress(strings[i].addresses[j]);
				file->WriteAddress(address, Endianess::Big);
			}

			file->GoToEnd();
		}
	}

	void SonicOffsetTable::addEntry(unsigned char c, size_t of) {
		SonicOffsetTableEntry entry(c, of);

		bool found=false;
		for (std::list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			if (entry.offset <= (*it).offset) {
				entries.insert(it, entry);
				found = true;
				break;
			}
		}

		if (!found) entries.push_back(entry);
	}


	void SonicOffsetTable::printList() {
		size_t index=0;
		for (std::list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			Error::AddMessage(Error::LogType::LOG, std::to_string((*it).code) + "  (" + std::to_string((*it).offset) + ") " + std::to_string(index));
			index++;
		}

		Error::AddMessage(Error::LogType::LOG, "Done");
	}

	void SonicOffsetTableEntry::write(File *file) {
		file->Write<u8>(code);
	}

	void SonicOffsetTable::write(File *file) {
		for (std::list<SonicOffsetTableEntry>::iterator it=entries.begin(); it!=entries.end(); it++) {
			(*it).write(file);
		}

		file->FixPadding(4);
	}

	void SonicOffsetTable::writeSize(File *file) {
		unsigned int total_entries=entries.size();
		file->Write<u32>(total_entries, Endianess::Big);
	}
}