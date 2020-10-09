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
	void SonicFrameValue::read(File *file, bool big_endian) {
		frame = file->Read<f32>();
		value = file->Read<f32>();
	}

	void SonicFrameValue::write(File *file) {
		file->Write(frame);
		file->Write(value);
	}

	void SonicFrameValueFloats::read(File *file, bool big_endian) {
		frame = file->Read<f32>();
		value = file->Read<glm::vec3>();
	}

	void SonicFrameValueFloats::write(File *file) {
		file->Write<f32>(frame);
		file->Write<glm::vec3>(value);
	}

	void SonicFrameValueFloatsGroup::read(File *file, bool big_endian) {
		frame = file->Read<f32>();
		flag = file->Read<u32>();
		unknown_1 = file->Read<f32>();
		unknown_2 = file->Read<f32>();
		unknown_3 = file->Read<f32>();
		unknown_4 = file->Read<f32>();
	}

	void SonicFrameValueFloatsGroup::write(File *file) {
		// FIXME
	}

	void SonicFrameValueAngles::read(File *file, bool big_endian) {
		frame = file->Read<u16>();
		value_x = file->Read<u16>();
		value_y = file->Read<u16>();
		value_z = file->Read<u16>();
	}

	void SonicFrameValueAngles::write(File *file) {
		file->Write<u16>(frame);
		file->Write<u16>(value_x);
		file->Write<u16>(value_y);
		file->Write<u16>(value_z);
	}

	void SonicFrameValueIntBeta::read(File *file, bool big_endian) {
		frame = file->Read<f32>();
		value = file->Read<u16>();
	}

	void SonicFrameValueIntBeta::write(File *file) {
		file->Write<f32>(frame);
		file->Write<u16>(value);
	}

	void SonicFrameValueInt::read(File *file, bool big_endian) {
		frame = file->Read<u16>();
		value = file->Read<u16>();
	}

	void SonicFrameValueInt::write(File *file) {
		file->Write<u16>(frame);
		file->Write<u16>(value);
	}

	void SonicMotionControl::read(File *file, bool big_endian) {
		type = file->Read<u32>();
		flag = file->Read<u32>();
		bone_index = file->Read<u32>();
		start_frame = file->Read<f32>();
		end_frame= file->Read<f32>();
		start_key_frame = file->Read<f32>();
		end_key_frame = file->Read<f32>();

		unsigned int element_count = file->Read<u32>();
		element_size = file->Read<u32>();
		size_t address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);

		std::string type_str="UNKNOWN";
		if (type ==      LIBS06_XNMOTION_TYPE_X_COORDINATE_LINEAR)      type_str  = "X Coordinate";
		else if (type == LIBS06_XNMOTION_TYPE_Y_COORDINATE_LINEAR)      type_str  = "Y Coordinate";
		else if (type == LIBS06_XNMOTION_TYPE_Z_COORDINATE_LINEAR)      type_str  = "Z Coordinate";
		else if (type == LIBS06_XNMOTION_TYPE_X_SCALE_LINEAR)           type_str  = "X Scale";
		else if (type == LIBS06_XNMOTION_TYPE_Y_SCALE_LINEAR)           type_str  = "Y Scale";
		else if (type == LIBS06_XNMOTION_TYPE_Z_SCALE_LINEAR)           type_str  = "Z Scale";
		else if (type == LIBS06_XNMOTION_TYPE_X_ANGLE_LINEAR)           type_str  = "X Angle";
		else if (type == LIBS06_XNMOTION_TYPE_Y_ANGLE_LINEAR)           type_str  = "Y Angle";
		else if (type == LIBS06_XNMOTION_TYPE_Z_ANGLE_LINEAR)           type_str  = "Z Angle";
		else if (type == LIBS06_XNMOTION_TYPE_X_ANGLE_BETA)             type_str  = "X Angle Beta";
		else if (type == LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA)             type_str  = "Y Angle Beta";
		else if (type == LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA)             type_str  = "Z Angle Beta";
		else if (type == LIBS06_XNMOTION_TYPE_COORDINATES_LINEAR)       type_str  = "Translation";
		else if (type == LIBS06_XNMOTION_TYPE_ANGLES_LINEAR)            type_str  = "Rotation";
		else {
			printf("Unknown type %x with element size %d.\n", type, element_size);
			getchar();
		}

		Error::printfMessage(Error::LogType::LOG, "MotionControl Type %s(%x) for Bone %d:", type_str.c_str(), type, bone_index);
		Error::printfMessage(Error::LogType::LOG, "  Flag: %d", flag);
		Error::printfMessage(Error::LogType::LOG, "  Start: %d End: %f Start Key: %f End Key: %f", start_frame, end_frame, start_frame, end_frame);
		Error::printfMessage(Error::LogType::LOG, "  Elements: %d(%d) Elements Address: %d", element_count, element_size, address);

		if (element_size == 24) {
			for (size_t i=0; i<element_count; i++) {
				file->SetAddress(address + i*element_size);
				SonicFrameValueFloatsGroup *frame_value=new SonicFrameValueFloatsGroup();
				frame_value->read(file, big_endian);
				frame_values_floats_groups.push_back(frame_value);
			}
		}
		else if (element_size == 16) {
			for (size_t i=0; i<element_count; i++) {
				file->SetAddress(address + i*element_size);
				SonicFrameValueFloats *frame_value=new SonicFrameValueFloats();
				frame_value->read(file, big_endian);
				frame_values_floats.push_back(frame_value);
			}
		}
		else if (element_size == 8) {
			for (size_t i=0; i<element_count; i++) {
				file->SetAddress(address + i*element_size);
				if (type == LIBS06_XNMOTION_TYPE_ANGLES_LINEAR) {
					SonicFrameValueAngles *frame_value=new SonicFrameValueAngles();
					frame_value->read(file, big_endian);
					frame_values_angles.push_back(frame_value);
				}
				else if ((type == LIBS06_XNMOTION_TYPE_X_ANGLE_BETA) || 
					(type == LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA) || 
					(type == LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA)) {

					SonicFrameValueIntBeta *frame_value_int_beta=new SonicFrameValueIntBeta();
					frame_value_int_beta->read(file, big_endian);
					frame_values_int_beta.push_back(frame_value_int_beta);
				}
				else {
					SonicFrameValue *frame_value=new SonicFrameValue();
					frame_value->read(file, big_endian);
					frame_values.push_back(frame_value);
				}
			}
		}
		else if (element_size == 4) {
			for (size_t i=0; i<element_count; i++) {
				file->SetAddress(address + i*element_size);
				SonicFrameValueInt *frame_value_int=new SonicFrameValueInt();
				frame_value_int->read(file, big_endian);
				frame_values_int.push_back(frame_value_int);
			}
		}
	}

	
	void SonicMotionControl::writeFrameValues(File *file) {
		frame_value_address=file->GetCurrentAddress();

		if (element_size == 16) {
			for (size_t i=0; i<frame_values_floats.size(); i++) {
				frame_values_floats[i]->write(file);
			}
		}
		else if (element_size == 8) {
			if (type == LIBS06_XNMOTION_TYPE_ANGLES_LINEAR) {
				for (size_t i=0; i<frame_values_angles.size(); i++) {
					frame_values_angles[i]->write(file);
				}
			}
			else if ((type == LIBS06_XNMOTION_TYPE_X_ANGLE_BETA) || 
				(type == LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA) || 
				(type == LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA)) {

				for (size_t i=0; i<frame_values_int_beta.size(); i++) {
					frame_values_int_beta[i]->write(file);
				}
			}
			else {
				for (size_t i=0; i<frame_values.size(); i++) {
					frame_values[i]->write(file);
				}
			}
		}
		else if (element_size == 4) {
			for (size_t i=0; i<frame_values_int.size(); i++) {
				frame_values_int[i]->write(file);
			}
		}
		else {
		}
	}

	void SonicMotionControl::write(File *file) {
		file->Write<u32>(type);
		file->Write<u32>(flag);
		file->Write<u32>(bone_index);
		file->Write<f32>(start_frame);
		file->Write<f32>(end_frame);
		file->Write<f32>(start_key_frame);
		file->Write<f32>(end_key_frame);
		
		unsigned int element_count=0;
		if (element_size == 16)     element_count=frame_values_floats.size();
		else if (element_size == 8) {
			if (type == LIBS06_XNMOTION_TYPE_ANGLES_LINEAR) element_count=frame_values_angles.size();
			else if ((type == LIBS06_XNMOTION_TYPE_X_ANGLE_BETA) || (type == LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA) || (type == LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA)) element_count=frame_values_int_beta.size();
			else element_count=frame_values.size();
		}
		else if (element_size == 4) element_count=frame_values_int.size();
		file->Write<u32>(element_count);
		file->Write<u32>(element_size);
		file->WriteAddressFileEndianess(frame_value_address);
	}


	void SonicXNMotion::read(File *file) {
		SonicXNSection::read(file);
		size_t table_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		file->SetAddress(table_address);

		flag = file->Read<u32>();
		start_frame = file->Read<f32>();
		end_frame = file->Read<f32>();
		unsigned int motion_control_count = file->Read<u32>();
		size_t motion_control_address = file->ReadAddressFileEndianess(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		fps = file->Read<f32>();

		printf("Animation (%d) found with %f frames at %f FPS. Total MotionControls %d\n", flag, end_frame, fps, motion_control_count);

		for (size_t i=0; i<motion_control_count; i++) {
			file->SetAddress(motion_control_address + 40*i);

			SonicMotionControl *motion_control = new SonicMotionControl();
			motion_control->read(file, big_endian);
			motion_controls.push_back(motion_control);
		}
	}

	void SonicXNMotion::writeBody(File *file) {
		size_t header_address=file->GetCurrentAddress();
		file->FixPadding(16);

		for (size_t i=0; i<motion_controls.size(); i++) {
			motion_controls[i]->writeFrameValues(file);
		}

		size_t motion_control_address=file->GetCurrentAddress();
		for (size_t i=0; i<motion_controls.size(); i++) {
			motion_controls[i]->write(file);
		}

		unsigned int motion_control_count=motion_controls.size();
		size_t motion_header_address=file->GetCurrentAddress();
		file->Write<u32>(flag);
		file->Write<f32>(start_frame);
		file->Write<f32>(end_frame);
		file->Write<u32>(motion_control_count);
		file->WriteAddressFileEndianess(motion_control_address);
		file->Write(fps);

		file->SetAddress(header_address);
		file->WriteAddressFileEndianess(motion_header_address); // TODO File tables stuff this one is false: , false
		file->GoToEnd();
	}

	glm::vec3 SonicMotionControl::getFrameVector(float frame, glm::vec3 reference) {
		if (element_size == 16) {
			SonicFrameValueFloats *frame_value_prev=NULL;
			SonicFrameValueFloats *frame_value_next=NULL;

			for (size_t i=0; i<frame_values_floats.size(); i++) {
				if (frame_values_floats[i]->frame <= frame) {
					frame_value_prev = frame_values_floats[i];
				}
				else {
					frame_value_next = frame_values_floats[i];
					break;
				}
			}

			if (frame_value_prev && frame_value_next) {
				float time_diff  = frame_value_next->frame - frame_value_prev->frame;
				glm::vec3 value_diff = frame_value_next->value - frame_value_prev->value;
				float scale      = (frame - frame_value_prev->frame) / time_diff;

				return frame_value_prev->value + value_diff*scale;
			}
			else if (frame_value_prev) {
				return frame_value_prev->value;
			}
			else if (frame_value_next) {
				return frame_value_next->value;
			}
			else {
				return reference;
			}
		}
		else if (element_size == 8) {
			SonicFrameValueAngles *frame_value_prev=NULL;
			SonicFrameValueAngles *frame_value_next=NULL;
			
			for (size_t i=0; i<frame_values_angles.size(); i++) {
				if (frame_values_angles[i]->frame <= frame) {
					frame_value_prev = frame_values_angles[i];
				}
				else {
					frame_value_next = frame_values_angles[i];
					break;
				}
			}

			if (frame_value_prev && frame_value_next) {
				float range=65535.0f;
				float end=0.0f;
				float start=0.0f;

				float value_x=0;
				float value_y=0;
				float value_z=0;

				for (size_t x=0; x<3; x++) {
					float time_diff  = frame_value_next->frame - frame_value_prev->frame;
					float scale      = (frame - frame_value_prev->frame) / time_diff;

					if (x==0) {
						end = frame_value_next->value_x;
						start = frame_value_prev->value_x;
					}
					if (x==1) {
						end = frame_value_next->value_y;
						start = frame_value_prev->value_y;
					}
					if (x==2) {
						end = frame_value_next->value_z;
						start = frame_value_prev->value_z;
					}
	
					float difference = abs(end - start);
					if (difference > range/2.0f) {
						if (end > start) {
							start += range;
						}
						else {
							end += range;
						}
					}
					
					float result=(start + ((end - start) * scale));
					if (x==0) value_x = result;
					if (x==1) value_y = result;
					if (x==2) value_z = result;
				}

				if (value_x > range) value_x -= range;
				if (value_x < 0.0f)  value_x += range;

				if (value_y > range) value_y -= range;
				if (value_y < 0.0f)  value_y += range;

				if (value_z > range) value_z -= range;
				if (value_z < 0.0f)  value_z += range;

				return glm::vec3(value_x, value_y, value_z);
			}
			else if (frame_value_prev) {
				return glm::vec3(frame_value_prev->value_x, frame_value_prev->value_y, frame_value_prev->value_z);
			}
			else if (frame_value_next) {
				return glm::vec3(frame_value_next->value_x, frame_value_next->value_y, frame_value_next->value_z);
			}
			else {
				return reference;
			}
		}
		else {
			return reference;
		}
	}


	float SonicMotionControl::getFrameValue(float frame, float reference) {
		if (element_size == 8) {
			if ((type == LIBS06_XNMOTION_TYPE_X_ANGLE_BETA) || 
				(type == LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA) || 
				(type == LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA)) {

				SonicFrameValueIntBeta *frame_value_prev=NULL;
				SonicFrameValueIntBeta *frame_value_next=NULL;
			
				for (size_t i=0; i<frame_values_int_beta.size(); i++) {
					if (frame_values_int_beta[i]->frame <= frame) {
						frame_value_prev = frame_values_int_beta[i];
					}
					else {
						frame_value_next = frame_values_int_beta[i];
						break;
					}
				}

				if (frame_value_prev && frame_value_next) {
					float range=65535.0f;
					float time_diff  = frame_value_next->frame - frame_value_prev->frame;
					float end        = frame_value_next->value; 
					float start      = frame_value_prev->value;
					float scale      = (frame - frame_value_prev->frame) / time_diff;


					float difference = abs(end - start);
					if (difference > range/2.0f) {
						if (end > start) {
							start += range;
						}
						else {
							end += range;
						}
					}

					float value = (start + ((end - start) * scale));
					if (value > range) value -= range;
					if (value < 0.0f)  value += range;
					return value;
				}
				else if (frame_value_prev) {
					return frame_value_prev->value;
				}
				else if (frame_value_next) {
					return frame_value_next->value;
				}
				else {
					return reference;
				}
			}
			else {
				SonicFrameValue *frame_value_prev=NULL;
				SonicFrameValue *frame_value_next=NULL;

				for (size_t i=0; i<frame_values.size(); i++) {
					if (frame_values[i]->frame <= frame) {
						frame_value_prev = frame_values[i];
					}
					else {
						frame_value_next = frame_values[i];
						break;
					}
				}

				if (frame_value_prev && frame_value_next) {
					float time_diff  = frame_value_next->frame - frame_value_prev->frame;
					float value_diff = frame_value_next->value - frame_value_prev->value;
					float scale      = (frame - frame_value_prev->frame) / time_diff;

					return frame_value_prev->value + value_diff*scale;
				}
				else if (frame_value_prev) {
					return frame_value_prev->value;
				}
				else if (frame_value_next) {
					return frame_value_next->value;
				}
				else {
					return reference;
				}
			}
		}
		else if (element_size == 4) {
			SonicFrameValueInt *frame_value_prev=NULL;
			SonicFrameValueInt *frame_value_next=NULL;
			
			for (size_t i=0; i<frame_values_int.size(); i++) {
				if (frame_values_int[i]->frame <= frame) {
					frame_value_prev = frame_values_int[i];
				}
				else {
					frame_value_next = frame_values_int[i];
					break;
				}
			}

			if (frame_value_prev && frame_value_next) {
				float range=65535.0f;
				float time_diff  = frame_value_next->frame - frame_value_prev->frame;
				float end        = frame_value_next->value; 
				float start      = frame_value_prev->value;
				float scale      = (frame - frame_value_prev->frame) / time_diff;


				float difference = abs(end - start);
				if (difference > range/2.0f) {
					if (end > start) {
						start += range;
					}
					else {
						end += range;
					}
				}

				float value = (start + ((end - start) * scale));
				if (value > range) value -= range;
				if (value < 0.0f)  value += range;
				return value;
			}
			else if (frame_value_prev) {
				return frame_value_prev->value;
			}
			else if (frame_value_next) {
				return frame_value_next->value;
			}
			else {
				return reference;
			}
		}
		else {
			return reference;
		}
	}

	void SonicMotionControl::optimize() {
		float start=999999.0f;
		float end=0.0f;

		std::vector<SonicFrameValue *>::iterator it=frame_values.begin();
		while (it!=frame_values.end()) {
			std::vector<SonicFrameValue *>::iterator next_element = it+1;
			std::vector<SonicFrameValue *>::iterator next_element_2 = it+2;

			if (start > (*it)->frame) start = (*it)->frame;
			if (end < (*it)->frame) end = (*it)->frame;
			
			if (next_element != frame_values.end()) {
				if (next_element_2 != frame_values.end()) {
					if (((*it)->value == (*next_element)->value) && ((*it)->value == (*next_element_2)->value)) {
						delete (*next_element);
						frame_values.erase(next_element);
						continue;
					}
				}
				else {
					if ((*it)->value == (*next_element)->value) {
						delete (*next_element);
						frame_values.erase(next_element);
						continue;
					}
				}
			}
			
			it++;
		}

		if (frame_values.size() == 1) flag=0x20004;

		if (!frame_values.size()) {
			start_key_frame = 0.0f;
			end_key_frame   = end_frame;
		}
		else {
			start_key_frame = start;
			end_key_frame   = end;
		}
	}
	
	void SonicMotionControl::optimizeInt() {
		float start=999999.0f;
		float end=0.0f;
		
		std::vector<SonicFrameValueInt *>::iterator it=frame_values_int.begin();
		while (it!=frame_values_int.end()) {
			std::vector<SonicFrameValueInt *>::iterator next_element = it+1;
			std::vector<SonicFrameValueInt *>::iterator next_element_2 = it+2;
			
			if (start > (*it)->frame) start = (*it)->frame;
			if (end < (*it)->frame) end = (*it)->frame;
			
			if (next_element != frame_values_int.end()) {
				if (next_element_2 != frame_values_int.end()) {
					if (((*it)->value == (*next_element)->value) && ((*it)->value == (*next_element_2)->value)) {
						delete (*next_element);
						frame_values_int.erase(next_element);
						continue;
					}
				}
				else {
					if ((*it)->value == (*next_element)->value) {
						delete (*next_element);
						frame_values_int.erase(next_element);
						continue;
					}
				}
			}
			
			it++;
		}

		if (frame_values_int.size() == 1) flag=0x20004;

		if (!frame_values_int.size()) {
			start_key_frame = 0.0f;
			end_key_frame   = end_frame;
		}
		else {
			start_key_frame = start;
			end_key_frame   = end;
		}
	}

	
	void SonicMotionControl::optimizeAngles() {
		float start=999999.0f;
		float end=0.0f;

		std::vector<SonicFrameValueAngles *>::iterator it=frame_values_angles.begin();
		while (it!=frame_values_angles.end()) {
			if (start > (*it)->frame) start = (*it)->frame;
			if (end < (*it)->frame) end = (*it)->frame;
			it++;
		}

		if (frame_values_angles.size() == 1) flag=0x20004;

		if (!frame_values_angles.size()) {
			start_key_frame = 0.0f;
			end_key_frame   = end_frame;
		}
		else {
			start_key_frame = start;
			end_key_frame   = end;
		}
	}

	void SonicMotionControl::setScale(float scale) {
		if ((type == LIBS06_XNMOTION_TYPE_X_COORDINATE_LINEAR) || 
			(type == LIBS06_XNMOTION_TYPE_Y_COORDINATE_LINEAR) || 
			(type == LIBS06_XNMOTION_TYPE_Z_COORDINATE_LINEAR) ||
			(type == LIBS06_XNMOTION_TYPE_COORDINATES_LINEAR)) {

			for (size_t i=0; i<frame_values.size(); i++) {
				frame_values[i]->value *= scale;
			}

			for (size_t i=0; i<frame_values_floats.size(); i++) {
				frame_values_floats[i]->value = frame_values_floats[i]->value * scale;
			}
		}
	}


	SonicMotionControl *SonicXNMotion::getMotionControl(unsigned int type, unsigned int bone_index) {
		for (size_t i=0; i<motion_controls.size(); i++) {
			if ((motion_controls[i]->type == type) && (motion_controls[i]->bone_index == bone_index)) return motion_controls[i];
		}

		return NULL;
	}

	SonicMotionControl *SonicXNMotion::getPositionMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_COORDINATES_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAnglesMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_ANGLES_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getPositionXMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_X_COORDINATE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getPositionYMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Y_COORDINATE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getPositionZMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Z_COORDINATE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleXMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_X_ANGLE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleYMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Y_ANGLE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleZMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Z_ANGLE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleBetaXMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_X_ANGLE_BETA, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleBetaYMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Y_ANGLE_BETA, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getAngleBetaZMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Z_ANGLE_BETA, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getScaleXMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_X_SCALE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getScaleYMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Y_SCALE_LINEAR, bone_index);
	}

	SonicMotionControl *SonicXNMotion::getScaleZMotionControl(unsigned int bone_index) {
		return getMotionControl(LIBS06_XNMOTION_TYPE_Z_SCALE_LINEAR, bone_index);
	}


	void SonicXNMotion::pushMotionControl(SonicMotionControl *motion_control) {
		for (std::vector<SonicMotionControl *>::iterator it=motion_controls.begin(); it!=motion_controls.end(); it++) {
			if ((*it)->bone_index > motion_control->bone_index) {
				motion_controls.insert(it, motion_control);
				return;
			}
		}

		motion_controls.push_back(motion_control);
	}

	void SonicXNMotion::deleteMotionControl(SonicMotionControl *motion_control) {
		for (std::vector<SonicMotionControl *>::iterator it=motion_controls.begin(); it!=motion_controls.end(); it++) {
			if ((*it) == motion_control) {
				motion_controls.erase(it);
				delete motion_control;
				return;
			}
		}
	}

	void SonicXNMotion::clearMotionControls() {
		for (size_t i=0; i<motion_controls.size(); i++) {
			delete motion_controls[i];
		}
		motion_controls.clear();
	}

	void SonicXNMotion::optimizeMotionControls(unsigned int bone_index, float bone_x, float bone_y, float bone_z, 
											   float bone_scale_x, float bone_scale_y, float bone_scale_z, 
											   unsigned short bone_rot_x, unsigned short bone_rot_y, unsigned short bone_rot_z) {
		
		SonicMotionControl *pos_x_motion_control = getPositionXMotionControl(bone_index);
		SonicMotionControl *pos_y_motion_control = getPositionYMotionControl(bone_index);
		SonicMotionControl *pos_z_motion_control = getPositionZMotionControl(bone_index);
		SonicMotionControl *rot_x_motion_control = getAngleXMotionControl(bone_index);
		SonicMotionControl *rot_y_motion_control = getAngleYMotionControl(bone_index);
		SonicMotionControl *rot_z_motion_control = getAngleZMotionControl(bone_index);
		SonicMotionControl *sca_x_motion_control = getScaleXMotionControl(bone_index);
		SonicMotionControl *sca_y_motion_control = getScaleYMotionControl(bone_index);
		SonicMotionControl *sca_z_motion_control = getScaleZMotionControl(bone_index);

		if (pos_x_motion_control) pos_x_motion_control->optimize();
		if (pos_y_motion_control) pos_y_motion_control->optimize();
		if (pos_z_motion_control) pos_z_motion_control->optimize();
		if (rot_x_motion_control) rot_x_motion_control->optimizeInt();
		if (rot_y_motion_control) rot_y_motion_control->optimizeInt();
		if (rot_z_motion_control) rot_z_motion_control->optimizeInt();
		if (sca_x_motion_control) sca_x_motion_control->optimize();
		if (sca_y_motion_control) sca_y_motion_control->optimize();
		if (sca_z_motion_control) sca_z_motion_control->optimize();

		
		if (pos_x_motion_control && pos_y_motion_control && pos_z_motion_control) {
			bool flag_1   =  pos_x_motion_control->onlyOneFrameValue();
			bool flag_2   =  pos_y_motion_control->onlyOneFrameValue();
			bool flag_3   =  pos_z_motion_control->onlyOneFrameValue();
			float value_1 =  pos_x_motion_control->getFirstFrameValue();
			float value_2 =  pos_y_motion_control->getFirstFrameValue();
			float value_3 =  pos_z_motion_control->getFirstFrameValue();

			if (flag_1 && flag_2 && flag_3 && (value_1==bone_x) && (value_2==bone_y) && (value_3==bone_z)) {
				deleteMotionControl(pos_x_motion_control);
				deleteMotionControl(pos_y_motion_control);
				deleteMotionControl(pos_z_motion_control);
			}
		}

		if (rot_x_motion_control && rot_y_motion_control && rot_z_motion_control) {
			bool flag_1   =  rot_x_motion_control->onlyOneFrameValue();
			bool flag_2   =  rot_y_motion_control->onlyOneFrameValue();
			bool flag_3   =  rot_z_motion_control->onlyOneFrameValue();
			unsigned short value_1 =  rot_x_motion_control->getFirstFrameValueInt();
			unsigned short value_2 =  rot_y_motion_control->getFirstFrameValueInt();
			unsigned short value_3 =  rot_z_motion_control->getFirstFrameValueInt();

			if (flag_1 && flag_2 && flag_3 && (value_1==bone_rot_x) && (value_2==bone_rot_y) && (value_3==bone_rot_z)) {
				deleteMotionControl(rot_x_motion_control);
				deleteMotionControl(rot_y_motion_control);
				deleteMotionControl(rot_z_motion_control);
			}
		}

		if (sca_x_motion_control && sca_y_motion_control && sca_z_motion_control) {
			bool flag_1   =  sca_x_motion_control->onlyOneFrameValue();
			bool flag_2   =  sca_y_motion_control->onlyOneFrameValue();
			bool flag_3   =  sca_z_motion_control->onlyOneFrameValue();
			float value_1 =  sca_x_motion_control->getFirstFrameValue();
			float value_2 =  sca_y_motion_control->getFirstFrameValue();
			float value_3 =  sca_z_motion_control->getFirstFrameValue();

			if (flag_1 && flag_2 && flag_3 && (value_1==bone_scale_x) && (value_2==bone_scale_y) && (value_3==bone_scale_z)) {
				deleteMotionControl(sca_x_motion_control);
				deleteMotionControl(sca_y_motion_control);
				deleteMotionControl(sca_z_motion_control);
			}
		}
		
	}

	void SonicXNMotion::updateScaleMod(SonicXNObject *object) {
		for (size_t i=0; i<motion_controls.size(); i++) {
			motion_controls[i]->setScale(object->bones[motion_controls[i]->bone_index]->scale_animation_mod);
		}
	}
}