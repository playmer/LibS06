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
	void SonicOldMaterialTable::read(File *file, XNFileMode file_mode, bool big_endian) {
		unsigned short flag_1 = file->Read<u16>();
		unsigned short flag_2 = file->Read<u16>();
		size_t table_address = file->ReadAddressFileEndianess();

		Error::AddMessage(Error::LogType::LOG, "Reading Old Material Type " + ToString(flag_1) + " at " + ToString(table_address) + " with flag " + ToString(flag_2));

		file->SetAddress(table_address);

		if (flag_1 == 1) {
			file->SetAddress(table_address+52);
		}
		else if (flag_1 == 3) {
			file->SetAddress(table_address+96);
		}
		else if (flag_1 == 7) {
			file->SetAddress(table_address+96);
		}
		else if (flag_1 == 15) {
			file->SetAddress(table_address+96);
		}
		else if (flag_1 == 31) {
			file->SetAddress(table_address+96);
		}
		else {
			printf("Unknown Old Material Type %d at %d\n", (int)flag_1, (int)table_address);
			getchar();
		}

		texture_unit = file->Read<u32>();
		Error::AddMessage(Error::LogType::LOG, "  Resulting texture unit index is: " + ToString(texture_unit));
	}
};
