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

#pragma once

#include <string>

#define LIBS06_TEXT_ERROR_MESSAGE_NULL_FILE         "Trying to read text data from unreferenced file."
#define LIBS06_TEXT_ERROR_MESSAGE_WRITE_NULL_FILE   "Trying to write text data to an unreferenced file."

namespace LibS06 {
	class SonicTextEntry {
		protected:
			std::string id;
			std::string value;

			size_t file_address;
			size_t parameter_address;
		public:
			SonicTextEntry() {
			}

			void read(File *file);
			void write(File *file, SonicStringTable *string_table);
			void writeFixed(File *file);
			void writeValues(File *file);

			void setValue(std::string v) {
				value = v;
			}
	};

	class SonicText {
		protected:
			char *table;
			unsigned int table_size;
			std::string name;
			std::vector<SonicTextEntry *> entries;
			SonicStringTable string_table;
		public:
			SonicText(std::string filename);
			void read(File *file);
			void save(std::string filename);
			void write(File *file);
	};
};
