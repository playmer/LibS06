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
	enum class RotationOrder
	{
		XYZ,
		ZXY,
		XZY,
	};

	RotationOrder GetRotationOrder(unsigned int aFlag)
	{
		unsigned int rotation_flag = aFlag & 3840u;
		if (rotation_flag != 0u) 
		{
			if (rotation_flag != 256u) 
			{
				if (rotation_flag != 1024u)
				{
					return RotationOrder::XYZ;
				}
				else
				{
					return RotationOrder::ZXY;
				}
			}
			else
			{
				return RotationOrder::XZY;
			}
		}
		else
		{
			return RotationOrder::XYZ;
		}
	}
	
  glm::mat3 fromEulerAnglesZYX(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
      float fCos, fSin;

      fCos = glm::cos(fYAngle);
      fSin = glm::sin(fYAngle);
      glm::mat3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

      fCos = glm::cos(fPAngle);
      fSin = glm::sin(fPAngle);
      glm::mat3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

      fCos = glm::cos(fRAngle);
      fSin = glm::sin(fRAngle);
      glm::mat3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

      return kZMat*(kYMat*kXMat);
  }

	
  glm::mat3 fromEulerAnglesYXZ(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
      float fCos, fSin;

      fCos = glm::cos(fYAngle);
      fSin = glm::sin(fYAngle);
      glm::mat3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

      fCos = glm::cos(fPAngle);
      fSin = glm::sin(fPAngle);
      glm::mat3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

      fCos = glm::cos(fRAngle);
      fSin = glm::sin(fRAngle);
      glm::mat3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

      return kYMat*(kXMat*kZMat);
  }

	
  glm::mat3 fromEulerAnglesYZX(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
      float fCos, fSin;

      fCos = glm::cos(fYAngle);
      fSin = glm::sin(fYAngle);
      glm::mat3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

      fCos = glm::cos(fPAngle);
      fSin = glm::sin(fPAngle);
      glm::mat3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

      fCos = glm::cos(fRAngle);
      fSin = glm::sin(fRAngle);
      glm::mat3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

      return kYMat*(kZMat*kXMat);
  }


	glm::quat fromXYZInts(int rx, int ry, int rz) {
		float rot_x = rx * MathConstants::i32ToRadian;
		float rot_y = ry * MathConstants::i32ToRadian;
		float rot_z = rz * MathConstants::i32ToRadian;

		glm::mat3 mr = fromEulerAnglesZYX(rot_z, rot_y, rot_x);
		return glm::quat_cast(mr);
	}

	glm::quat fromXZYInts(int rx, int ry, int rz) {
		float rot_x = rx * MathConstants::i32ToRadian;
		float rot_y = ry * MathConstants::i32ToRadian;
		float rot_z = rz * MathConstants::i32ToRadian;

		glm::mat3 mr = fromEulerAnglesYZX(rot_z, rot_y, rot_x);
		return glm::quat_cast(mr);
	}

	glm::quat fromZXYInts(int rx, int ry, int rz) {
		float rot_x = rx * MathConstants::i32ToRadian;
		float rot_y = ry * MathConstants::i32ToRadian;
		float rot_z = rz * MathConstants::i32ToRadian;

		glm::mat3 mr = fromEulerAnglesYXZ(rot_z, rot_y, rot_x);
		return glm::quat_cast(mr);
	}
	
	glm::quat ReadRotation(File* aFile, unsigned int aFlag)
	{
		auto rotationOrder = GetRotationOrder(aFlag);
		auto rX = aFile->Read<i32>();
		auto rY = aFile->Read<i32>();
		auto rZ = aFile->Read<i32>();

		switch (rotationOrder)
		{
			case RotationOrder::XYZ: return fromXYZInts(rX, rY, rZ);
			case RotationOrder::XZY: return fromXZYInts(rX, rY, rZ);
			case RotationOrder::ZXY: return fromZXYInts(rX, rY, rZ);
			default: 
				Error::AddMessage(Error::LogType::ERROR, "Invalid rotation order passed to ReadRotation");
				return glm::quat(); // AddMessage will throw, but we need a return here for the compiler.
		}
	}
	

	void WriteRotation(File* aFile, unsigned int aFlag, glm::vec3& aRotation)
	{
		auto rotationOrder = GetRotationOrder(aFlag);
	}


	void SonicBone::read(File *file, bool big_endian, XNFileMode file_mode) {
		flag = file->Read<u32>();
		matrix_index= file->Read<u16>();
		parent_index = file->Read<u16>();
		child_index = file->Read<u16>();
		sibling_index = file->Read<u16>();
		translation = file->Read<glm::vec3>();
		orientation = ReadRotation(file, flag);
		scale = file->Read<glm::vec3>();
		matrix = file->Read<glm::mat4>();

		if (file_mode == MODE_GNO) {
			matrix = glm::transpose(matrix);
			matrix[3][3] = 1.0;
		}

		center = file->Read<glm::vec3>();
		radius = file->Read<u32>();

		if (file_mode != MODE_GNO) {
			user = file->Read<u32>();
			bounding_box = file->Read<glm::vec3>();
		}
		
		//unsigned int rotation_flag=flag & 3840u;
		//if (rotation_flag != 0u) 
		//{
		//	if (rotation_flag != 256u) 
		//	{
		//		if (rotation_flag != 1024u) 
		//			orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
		//		else 
		//			orientation.fromZXYInts(rotation_x, rotation_y, rotation_z);
		//	}
		//	else orientation.fromXZYInts(rotation_x, rotation_y, rotation_z);
		//}
		//else orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
	}

	void SonicBone::write(File *file) {
		file->Write<u32>(flag);
		file->Write<u16>(matrix_index);
		file->Write<u16>(parent_index);
		file->Write<u16>(child_index);
		file->Write<u16>(sibling_index);
		file->Write<glm::vec3>(translation);
		file->Write<u32>(rotation_x);
		file->Write<u32>(rotation_y);
		file->Write<u32>(rotation_z);
		file->Write<glm::vec3>(scale);
		file->Write<glm::mat4>(matrix);
		file->Write<glm::vec3>(center);
		file->Write<f32>(radius);
		file->Write<u32>(user);
		file->Write<glm::vec3>(bounding_box);
	}


	void SonicBone::setScale(float sca) {
		translation = translation * sca;
		center = center * sca;
		radius = radius * sca;
		scale_animation_mod *= sca;

		//glm::quat orientation;
		//unsigned int rotation_flag=flag & 3840u;
		//if (rotation_flag != 0u) {
		//	if (rotation_flag != 256u) {
		//		if (rotation_flag != 1024u) orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
		//		else orientation.fromZXYInts(rotation_x, rotation_y, rotation_z);
		//	}
		//	else orientation.fromXZYInts(rotation_x, rotation_y, rotation_z);
		//}
		//else orientation.fromXYZInts(rotation_x, rotation_y, rotation_z);
		//current_matrix.makeTransform(translation, scale, orientation);
	}

	void SonicBone::zero() {
		flag = 0x2001C6;
		translation = glm::vec3();
		rotation_x = 0;
		rotation_y = 0;
		rotation_z = 0;
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		parent_index = 0xFF;
		matrix_index = 0;

		for (size_t x=0; x<4; x++) {
			for (size_t y=0; y<4; y++) {
				if (x==y) matrix[x][y] = 1.0f;
				else matrix[x][y] = 0.0f;
			}
		}

		child_index = 0xFFFF;
		sibling_index = 0xFFFF;
	}
};
