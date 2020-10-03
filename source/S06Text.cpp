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
#include "S06Common.h"
#include "S06Text.h"

namespace LibS06 {
	SonicText::SonicText(std::string filename) {
		File file(filename, File::Style::Read);

		if (file.Valid()) {
			read(&file);
			file.Close();
		}
	}

	void SonicTextEntry::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_TEXT_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		size_t id_address = file->ReadAddress(Endianess::Big);
		size_t value_address = file->ReadAddress(Endianess::Big);

		file->SetAddress(id_address);
		id = file->ReadNullTerminatedString();

		value = "";
		for (size_t i=0; i<65535; i++) {
			file->SetAddress(value_address + i*2 + 1);
			unsigned char c = file->Read<u8>(Endianess::Big);
			if (c) value += c;
			else break;
		}
	}

	
	void SonicTextEntry::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_TEXT_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, id);
		file->WriteByte(0, 8);
	}

	void SonicTextEntry::writeFixed(File *file) {
		file->SetAddress(file_address + 4);
		file->WriteAddress(parameter_address, Endianess::Big);
	}

	void SonicTextEntry::writeValues(File *file) {
		parameter_address = file->GetCurrentAddress();

		for (size_t i=0; i<value.size(); i++) {
			unsigned char c=value[i];
			file->WriteByte(0, 1);
			file->Write(c, Endianess::Little);
		}

		file->WriteByte(0, 2);
	}


	void SonicText::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_TEXT_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		size_t address=0;

		file->SetRootNodeAddress(32);
		unsigned int file_size = file->Read<u32>(Endianess::Big);
		size_t banana_table_address = file->ReadAddress(Endianess::Big);
		table_size = file->Read<u32>(Endianess::Big);

		file->SetAddress(36);
		size_t name_address = file->Read<u32>(Endianess::Big);
		file->SetAddress(name_address);
		name = file->ReadNullTerminatedString();

		file->SetAddress(40);
		unsigned int entries_total = file->Read<u32>(Endianess::Big);

		for (size_t i=0; i<entries_total; i++) {
			file->SetAddress(44 + i*12);

			SonicTextEntry *entry=new SonicTextEntry();
			entry->read(file);
			entries.push_back(entry);
		}
		
		file->SetAddress(banana_table_address);
		table = new char[table_size];
		file->ReadStream((void*)table, table_size);
	}

	
	void SonicText::save(std::string filename) {
		File file(filename, File::Style::Write);

		if (file.Valid()) {
			write(&file);
			file.Close();
		}
	}

	void SonicText::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_TEXT_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->SetRootNodeAddress(32);

		file->WriteByte(0, 8);
		file->Write<u32>(table_size, Endianess::Big);
		file->WriteByte(0, 10);
		std::string header="1BBINA";
		file->WriteNullTerminatedString(header.c_str());
		file->FixPadding(32);

		// Data
		header="WTXT";
		file->WriteStream((void*)header.c_str(), 4);

		string_table.writeString(file, name);
		unsigned int entries_total=entries.size();
		file->Write<u32>(entries_total, Endianess::Big);
		
		for (size_t i=0; i<entries_total; i++) entries[i]->write(file, &string_table);
		for (size_t i=0; i<entries_total; i++) entries[i]->writeValues(file);
		for (size_t i=0; i<entries_total; i++) entries[i]->writeFixed(file);
		file->GoToEnd();

		string_table.write(file);
		file->GoToEnd();

		// End
		file->FixPadding(16);
		size_t table_address=file->GetCurrentAddress();
		file->WriteStream((void*)table, table_size);
		unsigned int file_size=file->GetFileSize();

		file->SetAddress(0);
		file->Write<u32>(file_size, Endianess::Big);
		file->WriteAddress(table_address, Endianess::Big);
	}

};