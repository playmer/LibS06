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
	void SonicPolygonPoint::read(File *file, bool big_endian, unsigned char format_flag) {
		position_index = file->Read<u16>();

		if (format_flag & 0x1) {
			color_index = file->Read<u16>();
		}

		if (format_flag & 0x2) {
			file->OffsetAddress(2);
			printf("The format flag for the triangle strip has 0x2 enabled, this has not been cracked yet. Report with .gno\n");
			getchar();
		}

		if (format_flag & 0x4) {
			normal_index = file->Read<u16>();
		}

		if (format_flag & 0x8) {
			file->OffsetAddress(2);
			printf("The format flag for the triangle strip has 0x8 enabled, this has not been cracked yet. Report with .gno\n");
			getchar();
		}

		if (format_flag & 0x10) {
			uv_index = file->Read<u16>();
		}

		if (format_flag & 0x20) {
			uv_index = file->Read<u16>();
			uv2_index = file->Read<u16>();
		}
	}

	void SonicPolygonTable::read(File *file, bool big_endian) {
		unsigned int table_count = file->Read<u32>();
		size_t table_address = file->ReadAddressFileEndianess();

		file->SetAddress(table_address);
		flag = file->Read<u32>();

		if (table_count == 4) {
			size_t index_table_address = file->ReadAddressFileEndianess();
			unsigned int index_table_size = file->Read<u32>();
			file->SetAddress(index_table_address);

			unsigned int table_flag_2=0;
			file->OffsetAddress(20);
			
			unsigned char strip_flag = file->Read<u8>();
			unsigned short face_total = 0;

			Error::AddMessage(Error::LogType::LOG, "Reading strip with flag " + ToString((int)strip_flag) + " at address " + ToString(file->GetCurrentAddress()));

			while (file->GetCurrentAddress() < (index_table_address+index_table_size)) {
				printf("Reading strip type at address %d\n", (int)file->GetCurrentAddress());
				unsigned char strip_type = file->Read<u8>();

				if (strip_type == 0x99) {
					face_total = file->Read<u16>();
		
					if ((unsigned int)strip_flag <= 53) {
						SonicPolygonPoint last_point_1;
						SonicPolygonPoint last_point_2;
						SonicPolygonPoint point;
						int count=0;

						for (size_t i=0; i<face_total; i++) {
							last_point_1 = last_point_2;
							last_point_2 = point;
						
							point.read(file, big_endian, strip_flag);
							count++;

							if ((point == last_point_1) || (point == last_point_2) || (last_point_1 == last_point_2)) {
								continue;
							}
			
							if (count >= 3) {
								if (count%2==1) {
									SonicPolygon *polygon=new SonicPolygon(point, last_point_2, last_point_1);
									faces.push_back(polygon);
								}
								else {
									SonicPolygon *polygon=new SonicPolygon(last_point_1, last_point_2, point);
									faces.push_back(polygon);
								}
							}

							if (point.position_index == (unsigned short)0xFFFF) {
								count = 0;
							}
						}
					}
					else {
						printf("Unknown Strip Flag %d at address %d\n", (int) strip_flag, (int)file->GetCurrentAddress());
						getchar();
						break;
					}
				}
				else if (strip_type == 0) {
					break;
				}
				else {
					printf("Unknown Strip Type %d at address %d\n", (int) strip_type, (int)file->GetCurrentAddress());
					getchar();
					break;
				}
			}
		}
		else if (table_count == 0) {
			unsigned char format_flag=0;
			unsigned int format_size=0;

			if (flag == 0x21000A) {
				format_flag = 1;
				format_size = 4;
			}
			else if (flag == 0x81000A) {
				format_flag = 16;
				format_size = 4;
			}
			else if (flag == 0xE1002A) {
				format_flag = 17;
				format_size = 6;
			}
			else {
				printf("Unknown Strip List Format Type %d at address %d\n", (int) flag, (int)file->GetCurrentAddress());
				getchar();
				return;
			}

			std::vector<unsigned short> strip_sizes;
			unsigned int total_strips = file->Read<u32>();
			size_t strips_address = file->ReadAddressFileEndianess();
			size_t faces_address = file->ReadAddressFileEndianess();

			for (size_t i=0; i<total_strips; i++) {
				file->SetAddress(strips_address+i*2);
				unsigned short strip_count = file->Read<u32>();
				strip_sizes.push_back(strip_count);
			}

			size_t additional_index=0;
			for (size_t m=0; m<strip_sizes.size(); m++) {
				SonicPolygonPoint last_point_1;
				SonicPolygonPoint last_point_2;
				SonicPolygonPoint point;
				int count=0;

				for (size_t i=additional_index; i<additional_index+strip_sizes[m]; i++) {
					last_point_1 = last_point_2;
					last_point_2 = point;
	
					file->SetAddress(faces_address + i*format_size);
					point.read(file, big_endian, format_flag);
					count++;

					if ((point == last_point_1) || (point == last_point_2) || (last_point_1 == last_point_2)) {
						continue;
					}
			
					if (count >= 3) {
						if (count%2==1) {
							SonicPolygon *polygon=new SonicPolygon(point, last_point_2, last_point_1);
							faces.push_back(polygon);
						}
						else {
							SonicPolygon *polygon=new SonicPolygon(last_point_1, last_point_2, point);
							faces.push_back(polygon);
						}
					}
				}

				additional_index += strip_sizes[m];
			}
		}
		else {
			printf("Unknown Polygon Table Type %d at address %d\n", (int) table_count, (int)table_address);
			getchar();
		}
	}
};
