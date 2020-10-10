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
#include <list>

#include "S06XnFile.h"

#include "File.hpp"

namespace LibS06 {
	SonicXNFile::SonicXNFile(XNFileMode file_mode_parameter) {
		info=NULL;
		offset_table=NULL;
		footer=NULL;
		end=NULL;
		big_endian=false;

		file_mode = file_mode_parameter;

		setHeaders();

		// Create Essential Sections
		info = new SonicXNInfo();
		info->setHeader(header_info);
		info->setFileMode(file_mode);
		info->setBigEndian(big_endian);

		offset_table = new SonicXNOffsetTable();
		offset_table->setHeader(LIBS06_XNSECTION_HEADER_OFFSET_TABLE);
		offset_table->setFileMode(file_mode);
		offset_table->setBigEndian(big_endian);

		footer=new SonicXNFooter();
		footer->setHeader(LIBS06_XNSECTION_HEADER_FOOTER);
		footer->setFileMode(file_mode);
		footer->setBigEndian(big_endian);

		end=new SonicXNEnd();
		end->setHeader(LIBS06_XNSECTION_HEADER_END);
		end->setFileMode(file_mode);
		end->setBigEndian(big_endian);
	}

	SonicXNFile::SonicXNFile(std::string filename, XNFileMode file_mode_parameter) {
		info=NULL;
		offset_table=NULL;
		footer=NULL;
		end=NULL;
		big_endian=false;

		folder = filename;
		size_t sz=folder.size();
		int last_slash=0;
		for (size_t i=0; i<sz; i++) {
			if ((folder[i] == '\\') || (folder[i] == '/')) last_slash=i;
		}
		if (last_slash) folder.erase(last_slash+1, folder.size()-last_slash-1);
		else folder="";
		
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		
		if ((filename.find(LIBS06_XNO_EXTENSION)      != std::string::npos) || (filename.find(LIBS06_XNM_EXTENSION)      != std::string::npos) || (file_mode_parameter == MODE_XNO)) {
			file_mode = MODE_XNO;
		}
		else if (filename.find(LIBS06_ZNO_EXTENSION) != std::string::npos  || (filename.find(LIBS06_ZNM_EXTENSION)      != std::string::npos) || (file_mode_parameter == MODE_ZNO)) {
			file_mode = MODE_ZNO;
		}
		else if (filename.find(LIBS06_INO_EXTENSION) != std::string::npos  || (filename.find(LIBS06_INM_EXTENSION)      != std::string::npos) || (file_mode_parameter == MODE_INO)) {
			file_mode = MODE_INO;
		}
		else if ((filename.find(LIBS06_GNO_EXTENSION) != std::string::npos) || (filename.find(LIBS06_GNM_EXTENSION) != std::string::npos) || (filename.find(LIBS06_GNA_EXTENSION) != std::string::npos) || (file_mode_parameter == MODE_GNO)) {
			file_mode = MODE_GNO;
		}
		else if (filename.find(LIBS06_ENO_EXTENSION) != std::string::npos || (file_mode_parameter == MODE_ENO)) {
			file_mode = MODE_ENO;
		}
		else if (file_mode_parameter == MODE_YNO) {
			file_mode = MODE_YNO;
		}

		// Will set the endianess
		setHeaders();

		auto endianessOfFile = big_endian ? Endianess::Big : Endianess::Little;
		
		mFileReading = std::make_unique<File>(filename, File::Style::Read, endianessOfFile);

		if (mFileReading->Valid()) {
			while (!end) {
				readSection(mFileReading.get());
			}
			mFileReading->Close();
		}

		SonicXNObject *object=getObject();
		if (object) {
			std::string name="Object";
			if (footer) name=footer->name;
			object->setNames(name);
		}

		PrintOffsetTables(mFileReading.get());
	}

	void SonicXNFile::PrintOffsetTables(File* aFile)
	{
		std::cout << "Root address: " << aFile->GetRootNodeAddress() << "\n\n";

		std::cout << "File Offset table \n";
		for (auto& address : offset_table->GetAddresses())
		{
			std::cout << address << "\n";
		}
		
    std::string line;
		std::cout << "Read Offset table \n";
		for (auto& [address, data] : aFile->GetAddressMap())
		{
      line += data.File;
      line += "(";
      line += std::to_string(data.Line);
      line += "): Address read in ";
      line += data.Function;
      line += " here.";

			std::cout << address << ": " << line << "\n";
			line.clear();
		}
		
		std::cout << "Offsets we didn't read: \n";
		for (auto& address : offset_table->GetAddresses())
		{
			if (aFile->GetAddressMap().count(address) == 0)
				std::cout << address << "\n";
		}

		std::cout << "Offsets read but weren't in the offset table: \n";
		for (auto& [address, data] : aFile->GetAddressMap())
		{
			auto it = std::find(offset_table->GetAddresses().begin(), offset_table->GetAddresses().end(), address);
			if (it == offset_table->GetAddresses().end())
			{
				line += data.File;
				line += "(";
				line += std::to_string(data.Line);
				line += "): Address read in ";
				line += data.Function;
				line += " here.";
				std::cout << address << ": " << line << "\n";
				line.clear();
			}
		}
	}

	void SonicXNFile::setHeaders() {
		switch (file_mode) {
			case MODE_XNO:
				header_info    = LIBS06_XNSECTION_HEADER_INFO_XNO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_XNO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_XNO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_XNO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_XNO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_XNO;
				break;

			case MODE_ZNO:
				header_info    = LIBS06_XNSECTION_HEADER_INFO_ZNO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_ZNO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_ZNO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_ZNO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_ZNO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_ZNO;
				break;

			case MODE_INO:
				header_info    = LIBS06_XNSECTION_HEADER_INFO_INO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_INO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_INO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_INO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_INO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_INO;
				break;

			case MODE_GNO:
				big_endian = true;
				header_info    = LIBS06_XNSECTION_HEADER_INFO_GNO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_GNO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_GNO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_GNO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_GNO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_GNO;
				break;

			case MODE_ENO:
				big_endian = true;
				header_info    = LIBS06_XNSECTION_HEADER_INFO_ENO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_ENO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_ENO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_ENO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_ENO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_ENO;
				break;

			case MODE_YNO:
				big_endian = true;
				header_info    = LIBS06_XNSECTION_HEADER_INFO_YNO;
				header_texture = LIBS06_XNSECTION_HEADER_TEXTURE_XNO;
				header_effect  = LIBS06_XNSECTION_HEADER_EFFECT_XNO;
				header_object  = LIBS06_XNSECTION_HEADER_OBJECT_XNO;
				header_bones   = LIBS06_XNSECTION_HEADER_BONES_XNO;
				header_motion  = LIBS06_XNSECTION_HEADER_MOTION_XNO;
				break;
		}
	}


	void SonicXNSection::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_XNINFO_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		
		head_address = file->GetCurrentAddress();
		section_size = file->Read<u32>() + 4;  // + 4 because the SonicXNSection isn't counted in the size of the inheritor.
	}

	void SonicXNSection::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_XNINFO_ERROR_MESSAGE_NULL_FILE);
			return;
		}
		
		head_address = file->GetCurrentAddress();
		file->WriteStream((void*)header.c_str(), 4);
		file->WriteByte(0, 4);

		writeBody(file);

		file->FixPadding(LIBS06_XNSECTION_PADDING);
		size_t bookmark=file->GetCurrentAddress();

		
		section_size = bookmark - head_address - LIBS06_XNSECTION_HEADER_SIZE;
		file->SetAddress(head_address + 4);
		file->Write<u32>(section_size);

		file->SetAddress(bookmark);
	}


	void SonicXNSection::goToEnd(File *file) {
		file->SetAddress(head_address + section_size);
	}


	void SonicXNInfo::read(File *file) {
		SonicXNSection::read(file);

		section_count = file->Read<u32>();
		unsigned int file_root_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		file->SetRootNodeAddress(file_root_address);
	}

	void SonicXNInfo::writeBody(File *file) {
		file->Write<u32>(section_count, Endianess::Big);
		unsigned int root_address=32;
		file->Write<u32>(root_address, Endianess::Big);
		file->WriteByte(0, 12);

		unsigned int unknown_value=1;
		file->Write<u32>(unknown_value, Endianess::Big);
	}


	void SonicXNInfo::writeFixed(File *file) {
		size_t address=file->GetCurrentAddress();

		file->SetAddress(head_address + 16);
		file->WriteAddress(offset_table_address, Endianess::Big);
		file->Write<u32>(offset_table_address_raw, Endianess::Big);
		file->Write<u32>(offset_table_size, Endianess::Big);

		file->SetAddress(address);
	}


	void SonicXNOffsetTable::read(File *file) {
		SonicXNSection::read(file);

		unsigned int offset_count = file->Read<u32>();
		file->OffsetAddress(4);

		for (size_t i=0; i<offset_count; i++) {
			size_t address = file->Read<u32>();
			addresses.push_back(address);
		}
	}

	void SonicXNOffsetTable::writeBody(File *file) {
		unsigned int offset_count=addresses.size();
		file->Write<u32>(offset_count);
		file->WriteByte(0, 4);

		for (size_t i=0; i<offset_count; i++) {
			size_t address=addresses[i];
			file->Write<u32>(address);
		}
	}

	
	void SonicXNFooter::read(File *file) {
		SonicXNSection::read(file);

		file->OffsetAddress(8);
		name = file->ReadNullTerminatedString();
	}

	void SonicXNFooter::writeBody(File *file) {
		file->WriteByte(0, 8);
		file->WriteNullTerminatedString(name.c_str());
	}


	SonicXNSection *SonicXNFile::readSection(File *file) {
		std::string identifier = file->ReadString(4);

		if (identifier == header_info) {
			info = new SonicXNInfo();
			info->setHeader(header_info);
			info->setFileMode(file_mode);
			info->setBigEndian(big_endian);
			info->read(file);
			info->goToEnd(file);
			
			for (size_t i=0; i<info->getSectionCount(); i++) {
				SonicXNSection *section=readSection(file);
				if (section) {
					section->goToEnd(file);
					sections.push_back(section);
				}
			}
		}
		else if (identifier == LIBS06_XNSECTION_HEADER_OFFSET_TABLE) {
			offset_table = new SonicXNOffsetTable();
			offset_table->setHeader(LIBS06_XNSECTION_HEADER_OFFSET_TABLE);
			offset_table->setFileMode(file_mode);
			offset_table->setBigEndian(big_endian);
			offset_table->read(file);
			offset_table->goToEnd(file);
		}
		else if (identifier == header_texture) {
			SonicXNTexture *section=new SonicXNTexture();
			section->setHeader(header_texture);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_effect) {
			SonicXNEffect *section=new SonicXNEffect();
			section->setHeader(header_effect);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_object) {
			SonicXNObject *section=new SonicXNObject(getTexture(), getEffect(), getBones());
			section->setHeader(header_object);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);

			return section;
		}
		else if (identifier == header_bones) {
			SonicXNBones *section=new SonicXNBones();
			section->setHeader(header_bones);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == header_motion) {
			SonicXNMotion *section=new SonicXNMotion();
			section->setHeader(header_motion);
			section->setFileMode(file_mode);
			section->setBigEndian(big_endian);
			section->read(file);
			return section;
		}
		else if (identifier == LIBS06_XNSECTION_HEADER_FOOTER) {
			footer=new SonicXNFooter();
			footer->setHeader(LIBS06_XNSECTION_HEADER_FOOTER);
			footer->setFileMode(file_mode);
			footer->setBigEndian(big_endian);
			footer->read(file);
			footer->goToEnd(file);
			return footer;
		}
		else if (identifier == LIBS06_XNSECTION_HEADER_END) {
			end=new SonicXNEnd();
			end->setHeader(LIBS06_XNSECTION_HEADER_END);
			end->setFileMode(file_mode);
			end->setBigEndian(big_endian);
			end->read(file);
			end->goToEnd(file);
			return end;
		}

		return NULL;
	}

	
	void SonicXNFile::save(std::string filename) {
		Endianess endianessOfFile;
		
		if ((filename.find(LIBS06_XNO_EXTENSION) != std::string::npos) || 
				(filename.find(LIBS06_XNM_EXTENSION) != std::string::npos) ||
				(filename.find(LIBS06_ZNO_EXTENSION) != std::string::npos) || 
				(filename.find(LIBS06_ZNM_EXTENSION) != std::string::npos) ||
				(filename.find(LIBS06_INO_EXTENSION) != std::string::npos) || 
				(filename.find(LIBS06_INM_EXTENSION) != std::string::npos))
			endianessOfFile = Endianess::Little;
		else if ((filename.find(LIBS06_GNO_EXTENSION) != std::string::npos) || 
						 (filename.find(LIBS06_GNM_EXTENSION) != std::string::npos) || 
						 (filename.find(LIBS06_GNA_EXTENSION) != std::string::npos) ||
						 (filename.find(LIBS06_ENO_EXTENSION) != std::string::npos))
			endianessOfFile = Endianess::Big;

		File file(filename, File::Style::Write, endianessOfFile);

		if (file.Valid()) {
			write(&file);
			file.Close();
		}
	}

	void SonicXNFile::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_XNINFO_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		file->SetRootNodeAddress(32);

		if (info) {
			info->setSectionCount(sections.size());
			info->write(file);
		}

		for (size_t i=0; i<sections.size(); i++) {
			sections[i]->write(file);
		}

		if (offset_table) {
			offset_table->clear();
			
			file->SortAddressTable();
			for (auto& item : file->GetAddressTable())
			{
				offset_table->push(item);
			}

			offset_table->write(file);

			info->setOffsetTableAddress(offset_table->getAddress());
			info->setOffsetTableSize(offset_table->getSectionSize() + 8);

			info->writeFixed(file);
		}

		if (footer) footer->write(file);
		if (end) end->write(file);
	}

	void SonicXNFile::setFileMode(XNFileMode target_file_mode) {
		SonicXNObject  *object=getObject();
		SonicXNTexture *texture=getTexture();
		SonicXNEffect  *effect=getEffect();
		SonicXNBones   *bones=getBones();
		SonicXNMotion  *motion=getMotion();

		file_mode = target_file_mode;

		setHeaders();

		if (info) {
			info->setHeader(header_info);
			info->setFileMode(file_mode);
		}

		if (object)  {
			object->setFileMode(file_mode);
			object->setHeader(header_object);
		}
		if (texture) {
			texture->setFileMode(file_mode);
			texture->setHeader(header_texture);
		}
		if (effect)  {
			effect->setFileMode(file_mode);
			effect->setHeader(header_effect);
		}
		if (bones)   {
			bones->setFileMode(file_mode);
			bones->setHeader(header_bones);
		}
		if (motion)  {
			motion->setFileMode(file_mode);
			motion->setHeader(header_motion);
		}

		if (offset_table) offset_table->setFileMode(file_mode);
		if (footer) footer->setFileMode(file_mode);
		if (end) end->setFileMode(file_mode);
	}


	void SonicXNFile::createTextureSection() {
		SonicXNTexture *section=new SonicXNTexture();
		section->setHeader(header_texture);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createEffectSection() {
		SonicXNEffect *section=new SonicXNEffect();
		section->setHeader(header_effect);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createBoneSection() {
		SonicXNBones *section=new SonicXNBones();
		section->setHeader(header_bones);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}

	void SonicXNFile::createObjectSection() {
		SonicXNObject *section=new SonicXNObject(getTexture(), getEffect(), getBones());
		section->setHeader(header_object);
		section->setFileMode(file_mode);
		section->setBigEndian(big_endian);
		sections.push_back(section);
	}
}