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
	void SonicTextureUnitZNO::read(File *file, bool big_endian) {
		flag = file->Read<u32>();
		index = file->Read<u32>();
		enviroment_mode = file->Read<u32>();
		offset = file->Read<glm::vec2>();
		file->OffsetAddress(4);
		scale = file->Read<glm::vec2>();
		wrap_s = file->Read<u32>();
		wrap_t = file->Read<u32>();
		lod_bias = file->Read<f32>();
	}

	void SonicTextureUnitZNO::write(File *file) {
		file->Write<u32>(flag);
		file->Write<u32>(index);
		file->Write<u32>(enviroment_mode);
		file->Write<glm::vec2>(offset);
		file->WriteByte(0, 4);
		file->Write<glm::vec2>(scale);
		file->Write<u32>(wrap_s);
		file->Write<u32>(wrap_t);
		file->Write<f32>(lod_bias);
		file->WriteByte(0, 20);
	}

	void SonicMaterialColor::read(File *file, XNFileMode file_mode, bool big_endian) {
		flag = file->Read<u32>();
		ambient  = file->Read<glm::vec4>();
		diffuse  = file->Read<glm::vec4>();
		specular = file->Read<glm::vec4>();
		emission = file->Read<glm::vec4>();
		shininess = file->Read<f32>();
		specular_intensity = file->Read<f32>();
	}

	void SonicMaterialColor::write(File *file, XNFileMode file_mode) {
		file->Write<u32>(flag);
		file->Write<glm::vec4>(ambient);
		file->Write<glm::vec4>(diffuse);
		file->Write<glm::vec4>(specular);
		file->Write<glm::vec4>(emission);
		file->Write<f32>(shininess);
		file->Write<f32>(specular_intensity);
	}

	bool SonicMaterialColor::compare(SonicMaterialColor *color) {
		if (flag != color->flag) return false;
		if (ambient != color->ambient) return false;
		if (diffuse != color->diffuse) return false;
		if (specular != color->specular) return false;
		if (emission != color->emission) return false;
		if (shininess != color->shininess) return false;
		if (specular_intensity != color->specular_intensity) return false;

		return true;
	}

	void SonicMaterialProperties::read(File *file, XNFileMode file_mode, bool big_endian) {
		file->ReadStream((void*)data, 28);
	}

	void SonicMaterialProperties::write(File *file, XNFileMode file_mode) {
		file->WriteStream((void*)data, 28);
	}


	void SonicTextureUnit::read(File *file, bool big_endian) {
		flag_f = file->Read<f32>();
		index = file->Read<u32>();
		flag = file->Read<u32>();
		file->OffsetAddress(4);
		flag_2_f = file->Read<f32>();
		file->OffsetAddress(4);
		flag_2 = file->Read<u32>();
		flag_3_f = file->Read<f32>();
		flag_3 = file->Read<u32>();
		Error::printfMessage(Error::LogType::LOG, "Instance Material: %f %d", flag_f, index);
	}

	void SonicTextureUnit::write(File *file) {
		file->Write<f32>(flag_f);
		file->Write<>(index);
		file->Write<>(flag);
		file->WriteByte(0, 4);
		file->Write<f32>(flag_2_f);
		file->WriteByte(0, 4);
		file->Write<>(flag_2);
		file->Write<f32>(flag_3_f);
		file->Write<>(flag_3);
		file->WriteByte(0, 12);
	}

	bool SonicTextureUnit::compare(SonicTextureUnit *t) {
		if (flag_f    != t->flag_f)   return false;
		if (index     != t->index)    return false;
		if (flag_2_f  != t->flag_2_f) return false;
		if (flag      != t->flag)     return false;
		if (flag_2    != t->flag_2)   return false;
		if (flag_3_f  != t->flag_3_f) return false;
		if (flag_3    != t->flag_3)   return false;
		return true;
	}

	void SonicMaterialTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		size_t table_address=0;
		colors=NULL;
		properties=NULL;

		if (file_mode == MODE_ENO) return;

		count = file->Read<u32>();
		table_address = file->ReadAddress(big_endian);
		file->SetAddress(table_address);

		data_block_1_length = 20;
		data_block_2_length = 16;

		size_t data_1_offset=0;
		size_t data_2_offset=0;
		size_t texture_units_offset=0;

		flag_table = file->Read<u32>();
		user_flag = file->Read<u32>();
		data_1_offset = file->ReadAddress(big_endian);
		data_2_offset = file->ReadAddress(big_endian);
		
		unsigned int texture_unit_zno_count=0;
		if (file_mode == MODE_ZNO) {
			texture_unit_flag = file->Read<u32>();
			texture_unit_zno_count = file->Read<u32>();
		}
		file->ReadAddress(texture_units_offset);
		if (file_mode == MODE_ZNO) {
			texture_unit_flag_2 = file->Read<u32>();
		}

		if (file_mode == MODE_ZNO) {
			file->SetAddress(data_1_offset);
			colors = new SonicMaterialColor();
			colors->read(file, file_mode, big_endian);

			file->SetAddress(data_2_offset);
			properties = new SonicMaterialProperties();
			properties->read(file, file_mode, big_endian);
		}
		else {
			file->SetAddress(data_1_offset);
			file->ReadStream((void*)first_floats, data_block_1_length*4);

			file->SetAddress(data_2_offset);
			file->ReadStream((void*)first_ints, data_block_2_length*4);
		}

		if (file_mode == MODE_ZNO) {
			if (texture_unit_zno_count) {
				for (size_t i=0; i<texture_unit_zno_count; i++) {
					file->SetAddress(texture_units_offset + i*64);
					SonicTextureUnitZNO *texture_unit = new SonicTextureUnitZNO();
					texture_unit->read(file, big_endian);
					texture_units_zno.push_back(texture_unit);
				}
			}
		}
		else if (count) {
			count -= 16;
			for (size_t i=0; i<count; i++) {
				file->SetAddress(texture_units_offset + i*48);
				SonicTextureUnit *texture_unit = new SonicTextureUnit();
				texture_unit->read(file, big_endian);
				texture_units.push_back(texture_unit);
			}
		}

		Error::AddMessage(Error::LogType::LOG, "Done with Material.");
	}

	void SonicMaterialTable::write(File *file, XNFileMode file_mode) {
		head_address = file->GetCurrentAddress();

		if (file_mode == MODE_ZNO) {
			file->Write<u32>(count, Endianess::Big);
			file->WriteAddress(table_address, Endianess::Big);
		}
		else {
			unsigned int file_count=16 + texture_units.size();
			if (!texture_units.size()) file->WriteByte(0, 8);
			else {
				file->Write<u32>(file_count, Endianess::Big);
				file->WriteAddress(table_address, Endianess::Big);
			}
		}
	}

	void SonicMaterialTable::writeTable(File *file, XNFileMode file_mode) {
		table_address = file->GetCurrentAddress();

		file->Write<u32>(flag_table, Endianess::Big);
		file->Write<u32>(user_flag, Endianess::Big);
		file->WriteAddress(data_block_1_address, Endianess::Big);
		file->WriteAddress(data_block_2_address, Endianess::Big);

		if (file_mode == MODE_ZNO) {
			file->Write<u32>(texture_unit_flag, Endianess::Big);
			unsigned int texture_unit_zno_count=texture_units_zno.size();
			file->Write<u32>(texture_unit_zno_count, Endianess::Big);
		}

		file->WriteAddress(texture_units_address, Endianess::Big);
		if (file_mode == MODE_ZNO) {
			file->Write<u32>(texture_unit_flag_2, Endianess::Big);
		}
		else {
			file->WriteByte(0, 12);
		}
	}

	void SonicMaterialTable::writeDataBlock1(File *file, XNFileMode file_mode) {
		data_block_1_address = file->GetCurrentAddress();
		if (file_mode == MODE_ZNO) {
			colors->write(file, file_mode);
		}
		else {
			file->WriteStream((void*)first_floats, data_block_1_length*4);
		}
	}

	void SonicMaterialTable::writeDataBlock2(File *file, XNFileMode file_mode) {
		data_block_2_address = file->GetCurrentAddress();
		if (file_mode == MODE_ZNO) {
			properties->write(file, file_mode);
		}
		else {
			file->WriteStream((void*)first_ints, data_block_2_length*4);
		}
	}

	void SonicMaterialTable::writeTextureUnits(File *file, XNFileMode file_mode) {
		texture_units_address = file->GetCurrentAddress();

		if (file_mode == MODE_ZNO) {
			for (size_t i=0; i<texture_units_zno.size(); i++) {
				texture_units_zno[i]->write(file);
			}
		}
		else {
			for (size_t i=0; i<texture_units.size(); i++) {
				texture_units[i]->write(file);
			}
		}
	}

	
	bool SonicMaterialTable::compareDataBlock1(SonicMaterialTable *table, XNFileMode file_mode) {
		if (file_mode == MODE_ZNO) {
			colors->compare(table->colors);
		}
		else {
			for (size_t i=0; i<data_block_1_length; i++) {
				if (first_floats[i] != table->first_floats[i]) {
					return false;
				}
			}
		}
		return true;
	}

	bool SonicMaterialTable::compareDataBlock2(SonicMaterialTable *table, XNFileMode file_mode) {
		for (size_t i=0; i<data_block_2_length; i++) {
			if (first_ints[i] != table->first_ints[i]) {
				return false;
			}
		}
		return true;
	}

	bool SonicMaterialTable::compareTextureUnits(SonicMaterialTable *table, XNFileMode file_mode) {
		if (texture_units.size() > table->texture_units.size()) return false;

		for (size_t index=0; index<texture_units.size(); index++) {
			if (!texture_units[index]->compare(table->texture_units[index])) {
				return false;
			}
		}

		return true;
	}
};
