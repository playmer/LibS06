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

#include "S06Set.h"

namespace LibS06 {
	SonicSet::SonicSet(std::string filename) {
		File file(filename, File::Style::Read);

		if (file.Valid()) {
			read(&file);
			file.Close();
		}
	}

	SonicSetObjectParameter::SonicSetObjectParameter(SonicSetObjectParameter *clone) {
		value_f = clone->value_f;
		value_s = clone->value_s;
		value_i = clone->value_i;
		value_v = clone->value_v;
		type    = clone->type;
	}

	void SonicSetObjectParameter::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		size_t address=0;
		unsigned int total=0;

		type = file->Read<u32>(Endianess::Big);

		// Boolean
		if (type == 0) {
			value_i = file->Read<u32>(Endianess::Big);
		}
		// Integer
		else if (type == 1) {
			value_i = file->Read<u32>(Endianess::Big);
		}
		// Float
		else if (type == 2) {
			value_f = file->Read<f32>(Endianess::Big);
		}
		// Read std::string
		else if (type == 3) {
			size_t offset_address=file->GetCurrentAddress();
			address = file->Read<u32>(Endianess::Big);
			total = file->Read<u32>(Endianess::Big);
			file->SetAddress(address);
			value_s = file->ReadNullTerminatedString();
		}
		// glm::vec3
		else if (type == 4) {
			value_v = file->Read<glm::vec3>(Endianess::Big);
		}
		// Target another object
		else if (type == 6) {
			value_i = file->Read<u32>(Endianess::Big);
		}
	}

	
	void SonicSetObjectParameter::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->GetCurrentAddress();
		file->Write<u32>(type, Endianess::Big);

		if (type == 0) {
			file->Write<u32>(value_i, Endianess::Big);
			file->WriteByte(0, 12);
		}
		else if (type == 1) {
			file->Write<u32>(value_i, Endianess::Big);
			file->WriteByte(0, 12);
		}
		else if (type == 2) {
			file->Write<f32>(value_f, Endianess::Big);
			file->WriteByte(0, 12);
		}
		else if (type == 3) {
			string_table->writeString(file, value_s);
			unsigned int total=1;
			unsigned int size=value_s.size()+1;
			file->Write<u32>(total, Endianess::Big);
			file->WriteByte(0, 4);
			file->Write<u32>(size, Endianess::Big);
		}
		else if (type == 4) {
			file->Write<glm::vec3>(value_v, Endianess::Big);
			file->WriteByte(0, 4);
		}
		else if (type == 6) {
			file->Write<u32>(value_i, Endianess::Big);
			file->WriteByte(0, 12);
		}
	}


	SonicSetObject::SonicSetObject(SonicSetObject *clone) {
		position = clone->position;
		rotation = clone->rotation;
		name = clone->name;
		type = clone->type;
		unknown = clone->unknown;
		unknown_2 = clone->unknown_2;

		for (size_t i=0; i<clone->parameters.size(); i++) {
			SonicSetObjectParameter *parameter = new SonicSetObjectParameter(clone->parameters[i]);
			parameters.push_back(parameter);
		}
	}

	
	void SonicSetObject::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();

		size_t parameter_address=0;

		size_t name_1_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		size_t name_2_offset_address=file->GetCurrentAddress();
		size_t name_2_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		unknown = file->Read<f32>(Endianess::Big);
		file->OffsetAddress(12);
		position = file->Read<glm::vec3>();
		unknown_2 = file->Read<f32>(Endianess::Big);
		rotation = file->Read<glm::quat>();
		unsigned int parameter_total = file->Read<u32>(Endianess::Big);

		if (parameter_total) {
			parameter_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		}

		file->SetAddress(name_1_address);
		name = file->ReadNullTerminatedString();
		file->SetAddress(name_2_address);
		type = file->ReadNullTerminatedString();

		if (!parameter_total) {

		}
		else {

		}

		for (size_t i=0; i<parameter_total; i++) {
			file->SetAddress(parameter_address + i*20);

			SonicSetObjectParameter *parameter = new SonicSetObjectParameter();
			parameter->read(file);
			parameters.push_back(parameter);
		}
	}


	
	void SonicSetObject::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->GetCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, name);
		string_table->writeString(file, type);
		file->Write<f32>(unknown, Endianess::Big);
		file->WriteByte(0, 12);
		file->Write<glm::vec3>(position, Endianess::Big);
		file->Write<f32>(unknown_2, Endianess::Big);
		file->Write<glm::quat>(rotation, Endianess::Big);
		unsigned int parameter_total=parameters.size();
		file->Write<u32>(parameter_total, Endianess::Big);
		file->WriteByte(0, 4);
	}

	void SonicSetObject::writeFixed(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->SetAddress(file_address+60);

		if (parameters.size() > 0) {
			file->WriteAddress(parameter_address, Endianess::Big);
		}
	}



	void SonicSetGroup::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();

		size_t name_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		size_t type_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		unsigned int values_total = file->Read<u32>(Endianess::Big);
		size_t values_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);

		file->SetAddress(name_address);
		name = file->ReadNullTerminatedString();

		file->SetAddress(type_address);
		type = file->ReadNullTerminatedString();

		for (unsigned int i=0; i<values_total; i++) {
			unsigned int value=0;
			file->SetAddress(values_address + i*8 + 4);
			value = file->Read<i32>(Endianess::Big);
			values.push_back(value);
		}

		Error::printfMessage(Error::LogType::LOG, "%s - %s:", name.c_str(), type.c_str());
	}

	
	
	void SonicSetGroup::write(File *file, SonicStringTable *string_table) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		size_t header_address=file->GetCurrentAddress();
		file_address = header_address;

		string_table->writeString(file, name);
		string_table->writeString(file, type);
		unsigned int values_total=values.size();
		file->Write<u32>(values_total, Endianess::Big);
		file->WriteByte(0, 4);
	}

	void SonicSetGroup::writeValues(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}
		
		parameter_address = file->GetCurrentAddress();
		for (size_t i=0; i<values.size(); i++) {
			unsigned int value=values[i];
			file->WriteByte(0, 4);
			file->Write<u32>(value, Endianess::Big);
		}
	}

	void SonicSetGroup::writeFixed(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		file->SetAddress(file_address + 12);

		if (values.size()) {
			file->WriteAddress(parameter_address, Endianess::Big);
		}
	}


	void SonicSet::read(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->GetCurrentAddress();
		size_t address=0;

		file->SetRootNodeAddress(32);
		unsigned int file_size = file->Read<u32>(Endianess::Big);
		size_t banana_table_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);
		table_size = file->Read<u32>(Endianess::Big);

		file->SetAddress(44);
		name = file->ReadNullTerminatedString();

		file->SetAddress(76);

		unsigned int object_total = file->Read<u32>(Endianess::Big);
		size_t object_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);

		unsigned int group_total = file->Read<u32>(Endianess::Big);
		size_t group_address = file->ReadAddress(Endianess::Big, __FILE__, __LINE__, __PRETTY_FUNCTION__);

		for (size_t i=0; i<object_total; i++) {
			file->SetAddress(object_address + i*64);
			SonicSetObject *object=new SonicSetObject();
			object->read(file);
			objects.push_back(object);
		}


		for (size_t i=0; i<group_total; i++) {
			file->SetAddress(group_address + i*16);
			SonicSetGroup *group=new SonicSetGroup();
			group->read(file);
			groups.push_back(group);

			std::vector<unsigned int> values = group->getValues();

			for (size_t j=0; j<values.size(); j++) {
				Error::printfMessage(Error::LogType::LOG, "  %d: %s", values[j], objects[values[j]]->getName().c_str());
			}
		}

		file->SetAddress(banana_table_address);
		file->ReadAddressTableBBIN(table_size);
	}

	
	void SonicSet::save(std::string filename) {
		File file(filename, File::Style::Write);

		if (file.Valid()) {
			write(&file);
			file.Close();
		}
	}

	void SonicSet::write(File *file) {
		if (!file) {
			Error::AddMessage(Error::LogType::NULL_REFERENCE, LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		string_table.clear();

		file->SetRootNodeAddress(32);

		file->WriteByte(0, 22);
		std::string header="1BBINA";
		file->WriteNullTerminatedString(header.c_str());
		file->FixPadding(44);
		file->WriteNullTerminatedString(name.c_str());
		file->FixPadding(76);

		unsigned int object_total=objects.size();
		file->Write<u32>(object_total, Endianess::Big);
		size_t object_address=file->GetCurrentAddress();
		file->WriteByte(0, 4);

		unsigned int group_total=groups.size();
		file->Write<u32>(group_total, Endianess::Big);
		size_t group_address=file->GetCurrentAddress();
		file->WriteByte(0, 4);

		size_t object_table_address=file->GetCurrentAddress();
		for (size_t i=0; i<object_total; i++) objects[i]->write(file, &string_table);
		for (size_t i=0; i<object_total; i++) {
			std::vector<SonicSetObjectParameter *> parameters=objects[i]->getParameters();
			objects[i]->setAddress(file->GetCurrentAddress());
			for (size_t j=0; j<parameters.size(); j++) {
				parameters[j]->write(file, &string_table);
			}
		}
		for (size_t i=0; i<object_total; i++) objects[i]->writeFixed(file);
		file->GoToEnd();

		size_t group_table_address=file->GetCurrentAddress();
		for (size_t i=0; i<group_total; i++) groups[i]->write(file, &string_table);
		for (size_t i=0; i<group_total; i++) groups[i]->writeValues(file);
		for (size_t i=0; i<group_total; i++) groups[i]->writeFixed(file);
		file->GoToEnd();

		string_table.write(file);
		file->FixPadding(4);

		file->SetAddress(object_address);
		file->WriteAddress(object_table_address, Endianess::Big);

		file->SetAddress(group_address);
		if (group_total) {
			file->WriteAddress(group_table_address, Endianess::Big);
		}
		
		file->GoToEnd();
		u32 table_address = file->GetCurrentAddress() - 32;
		file->SortAddressTable();

		file->WriteAddressTableBBIN();
		file->FixPadding(8);

		unsigned int file_size = file->GetFileSize();
		unsigned int table_size = (file_size - 32) - table_address;
		file->SetAddress(0);
		file->Write<u32>(file_size, Endianess::Big);
		file->Write<u32>(table_address, Endianess::Big);
		file->Write<u32>(table_size, Endianess::Big);
	}

	void SonicSet::fixDuplicateNames() {
		for (size_t i=0; i<objects.size(); i++) {
			bool found=true;
			std::string object_name = objects[i]->getName();
			size_t index = 0;

			while (found) {
				found = false;
				for (size_t j=0; j<i; j++) {
					if (objects[j]->getName() == object_name) {
						found = true;
						break;
					}
				}

				if (!found) {
					objects[i]->setName(object_name);
				}
				else {
					object_name = objects[i]->getType() + ToString(index);
					index++;
				}
			}
		}
	}
};