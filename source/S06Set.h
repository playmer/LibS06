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

#include "S06XnFile.h"

#include "S06Common.h"

#define LIBS06_SET_ERROR_MESSAGE_NULL_FILE       "Trying to read set data from unreferenced file."
#define LIBS06_SET_ERROR_MESSAGE_WRITE_NULL_FILE "Trying to write set data to an unreferenced file."

#define LIBS06_SET_OFFSET_ROOT                   0x4C
#define LIBS06_SET_OFFSET_STRING_OBJECT          0x41
#define LIBS06_SET_OFFSET_PARAMETER_OBJECT       0x4E

namespace LibS06 {
	enum class SetParameters
	{
		Boolean = 0,
		Integer = 1,
		Float = 2,
		String = 3,
		Vector3 = 4,
		Id = 6
	};


	class SonicSetObjectParameter {
		friend SonicSetObjectParameter;

		protected:
			unsigned int type;
			float value_f;
			std::string value_s;
			unsigned int value_i;
			glm::vec3 value_v;
		public:
			SonicSetObjectParameter() {
			}

			SonicSetObjectParameter(SonicSetObjectParameter *clone);

			void read(File *file);

			void write(File *file, SonicStringTable *string_table);

			unsigned int getType() {
				return type;
			}

			void setValueInt(unsigned int v) {
				value_i = v;
			}

			unsigned int getValueInt() {
				return value_i;
			}

			float getValueFloat() {
				return value_f;
			}

			std::string getValueString() {
				return value_s;
			}

			bool getValueBool() {
				return value_i;
			}

			glm::vec3 getValueVector() {
				return value_v;
			}
	};

	class SonicSetObject {
		friend SonicSetObject;

		protected:
			glm::vec3 position;
			glm::quat rotation;

			std::string name;
			std::string type;

			float unknown;
			float unknown_2;
			std::vector<SonicSetObjectParameter *> parameters;

			size_t file_address;
			size_t parameter_address;
		public:
			SonicSetObject() {
			}

			void read(File *file);
			SonicSetObject(SonicSetObject *clone);

			std::vector<SonicSetObjectParameter *> getParameters() {
				return parameters;
			}

			void setAddress(size_t v) {
				parameter_address = v;
			}

			void write(File *file, SonicStringTable *string_table);
			void writeFixed(File *file);

			void setPosition(glm::vec3 v) {
				position=v;
			}

			glm::vec3 getPosition() {
				return position;
			}

			void setRotation(glm::quat v) {
				rotation=v;
			}

			glm::quat getRotation() {
				return rotation;
			}

			std::string getType() {
				return type;
			}

			std::string getName() {
				return name;
			}

			void setName(std::string v) {
				name = v;
			}

			float getUnknown() {
				return unknown;
			}

			float getUnknown2() {
				return unknown_2;
			}
	};

	class SonicSetGroup {
		protected:
			std::string name;
			std::string type;
			std::vector<unsigned int> values;

			size_t file_address;
			size_t parameter_address;
		public:
			SonicSetGroup() {
			}
			void read(File *file);
			void write(File *file, SonicStringTable *string_table);
			void writeValues(File *file);
			void writeFixed(File *file);

			void clearValues() {
				values.clear();
			}

			std::vector<unsigned int> getValues() {
				return values;
			}
	};


	class SonicSet {
		protected:
			std::vector<SonicSetObject *> objects;
			std::vector<SonicSetGroup *>  groups;
			unsigned int table_size;
			std::string name;
			SonicStringTable string_table;
		public:
			SonicSet() {

			}

			SonicSet(std::string filename);
			void read(File *file);
			void save(std::string filename);
			void write(File *file);

			void setName(std::string v) {
				name = v;
			}

			std::vector<SonicSetObject *> getObjects() {
				return objects;
			}

			void addObject(SonicSetObject *object) {
				if (!object) return;

				objects.push_back(object);
			}

			void deleteObjects() {
				for (size_t i=0; i<objects.size(); i++) {
					delete objects[i];
				}

				objects.clear();
			}

			void fixDuplicateNames();

			void clearGroupIDs() {
				for (size_t i=0; i<groups.size(); i++) {
					groups[i]->clearValues();
				}
			}

			void deleteGroups() {
				for (size_t i=0; i<groups.size(); i++) {
					delete groups[i];
				}
				groups.clear();
			}
	};
};
