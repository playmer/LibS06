
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <type_traits>

#include "half.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/common.hpp"

namespace LibS06
{
  using i8 = char;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;

  using u8 = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  using u64 = std::uint64_t;

  using f32 = f32;
  using f64 = double;

  enum class Endianess
  {
    Little, 
    Big
  };

  bool IsBigEndian(void)
  {
      union {
          uint32_t i;
          char c[4];
      } bint = {0x01020304};

      return bint.c[0] == 1; 
  }

  Endianess SystemEndianess()
  {
    return IsBigEndian() ? Endianess::Big : Endianess::Little;
  }

  template <typename tType>
  void SwapEndian(tType &aValue, typename std::enable_if<std::is_arithmetic<tType>::value, std::nullptr_t>::type = nullptr) {
      union U {
          tType value;
          std::array<std::uint8_t, sizeof(tType)> raw;
      } source, destination;

      source.value = aValue;
      std::reverse_copy(source.raw.begin(), source.raw.end(), destination.raw.begin());
      aValue = destination.value;
  }

  class File
  {
  public:
    enum class Style
    {
      Read,
      Write
    };

    File(std::string aFile, Style aStyle, Endianess aEndianess = Endianess::Little)
      : mStyle{aStyle}
      , mFileDefaultEndianess{aEndianess}
    {
      switch (aStyle)
      {
        case Style::Read: mFile = fopen(aFile.c_str(), "rb"); break;
        case Style::Write: mFile = fopen(aFile.c_str(), "wb"); break;
      }

      if (nullptr == mFile)
      {
        std::string output = "Couldn't open " + aFile + " for " + (mStyle == Style::Read ? "Reading" : "Writing");
        std::cout << output;
        throw(output);
      }
    }

    bool Valid()
    {
      return nullptr != mFile;
    }

    void Close()
    {
      fclose(mFile);
      mFile = nullptr;
    }

    template <typename tType>
    tType Read(Endianess aEndianess)
    {
      const bool needEndianessSwap =  aEndianess != SystemEndianess();

      auto value = ReadData<tType>();

      if (needEndianessSwap)
        SwapEndian(value);
    
      return value;
    }
    
    f32 ReadHalf(Endianess aEndianessRead)
    {
      u16 value = Read<u16>(aEndianessRead);

      return half_to_float(value);
    }
    
    f32 ReadHalf()
    {
      return ReadHalf(mFileDefaultEndianess);
    }

    template <typename tType>
    tType Read()
    {
      return Read(mFileDefaultEndianess);
    }

    template <>
    glm::vec2 Read<glm::vec2>(Endianess aEndianessRead)
    {
      glm::vec3 value;
      value.x = Read<f32>(aEndianessRead);
      value.y = Read<f32>(aEndianessRead);
      return value;
    }

    template <>
    glm::vec3 Read<glm::vec3>(Endianess aEndianessRead)
    {
      glm::vec3 value;
      value.x = Read<f32>(aEndianessRead);
      value.y = Read<f32>(aEndianessRead);
      value.z = Read<f32>(aEndianessRead);

      return value;
    }

    template <>
    glm::vec4 Read<glm::vec4>(Endianess aEndianessRead)
    {
      glm::vec4 value;
      value.x = Read<f32>(aEndianessRead);
      value.y = Read<f32>(aEndianessRead);
      value.z = Read<f32>(aEndianessRead);
      value.w = Read<f32>(aEndianessRead);

      return value;
    }

    template <>
    glm::quat Read<glm::quat>(Endianess aEndianessRead)
    {
      glm::quat value;
      value.x = Read<f32>(aEndianessRead);
      value.y = Read<f32>(aEndianessRead);
      value.z = Read<f32>(aEndianessRead);
      value.w = Read<f32>(aEndianessRead);

      return value;
    }

    template <>
    glm::mat4 Read<glm::mat4>(Endianess aEndianessRead)
    {
      glm::mat4 value;
      value[0] = Read<glm::vec4>(aEndianessRead);
      value[1] = Read<glm::vec4>(aEndianessRead);
      value[2] = Read<glm::vec4>(aEndianessRead);
      value[3] = Read<glm::vec4>(aEndianessRead);
      return value;
    }

    template <>
    glm::vec2 Read<glm::vec2>()
    {
      return Read<glm::vec2>(mFileDefaultEndianess);
    }

    template <>
    glm::vec3 Read<glm::vec3>()
    {
      return Read<glm::vec3>(mFileDefaultEndianess);
    }

    template <>
    glm::vec4 Read<glm::vec4>()
    {
      return Read<glm::vec4>(mFileDefaultEndianess);
    }

    template <>
    glm::quat Read<glm::quat>()
    {
      return Read<glm::quat>(mFileDefaultEndianess);
    }

    template <>
    glm::mat4 Read<glm::mat4>()
    {
      return Read<glm::mat4>(mFileDefaultEndianess);
    }

    size_t ReadAddress(Endianess aEndianessRead)
    {
      static_assert(false, "Implement");
    }

    size_t ReadAddress(bool aBigEndian)
    {
      return ReadAddress(aBigEndian ? Endianess::Big : Endianess::Little);
    }
    
    size_t ReadAddressFileEndianess()
    {
      return ReadAddress(mFileDefaultEndianess);
    }

    void WriteAddress(size_t aAddress, Endianess aEndianessRead)
    {
      if (aEndianessRead == Endianess::Big) 
        mFinalAddressTable.push_back(GetCurrentAddress() - mRootNodeAddress);

      Write<u32>(aAddress);
    }
    
    void WriteAddressFileEndianess(size_t aAddress)
    {
      WriteAddress(aAddress, mFileDefaultEndianess);
    }

    //size_t WriteAddress(size_t aAddress)
    //{
    //  WriteAddress(aAddress, mFileDefaultEndianess);
    //}
    
    void WriteNullTerminatedString(char const* aString)
    {
      for (; '\0' != *aString; ++aString)
        Write<char>(*aString, mFileDefaultEndianess);

      Write<char>('\0', mFileDefaultEndianess);
    }

    std::string ReadString(size_t aSize)
    {
      std::string value;

      for (size_t i = 0; i < aSize; ++i)
        value += Read<char>();

      return value;
    }

    std::string ReadNullTerminatedString()
    {
      std::string value;
      auto character = Read<char>();

      while ('\n' != character);
      {
        value += character;
        character = Read<char>();
      } 

      return value;
    }

    void ReadStream(void* aStream, size_t aSize)
    {
      fread(aStream, 1, aSize, mFile);
    }
    
    void WriteStream(void const* aStream, size_t aSize)
    {
      fwrite(aStream, 1, aSize, mFile);
    }
    
    template <typename tType>
    void Write(tType const& aValue, Endianess aEndianessRead)
    {
      // So I don't have to do some sfinae nonsense to disable passing pointers in.
      static_assert(false == std::is_pointer_v<tType>);

      const bool needEndianessSwap =  aEndianessRead != SystemEndianess();
      auto value = aValue;

      if (needEndianessSwap)
        SwapEndian(value);

      WriteData(value);
    }

    template <>
    void Write<glm::vec2>(glm::vec2 const& aValue, Endianess aEndianessRead)
    {
      Write(aValue.x, aEndianessRead);
      Write(aValue.y, aEndianessRead);
    }

    template <>
    void Write<glm::vec3>(glm::vec3 const& aValue, Endianess aEndianessRead)
    {
      Write(aValue.x, aEndianessRead);
      Write(aValue.y, aEndianessRead);
      Write(aValue.z, aEndianessRead);
    }
    
    template <>
    void Write<glm::vec4>(glm::vec4 const& aValue, Endianess aEndianessRead)
    {
      Write(aValue.x, aEndianessRead);
      Write(aValue.y, aEndianessRead);
      Write(aValue.z, aEndianessRead);
      Write(aValue.w, aEndianessRead);
    }
    
    template <>
    void Write<glm::quat>(glm::quat const& aValue, Endianess aEndianessRead)
    {
      Write(aValue.x, aEndianessRead);
      Write(aValue.y, aEndianessRead);
      Write(aValue.z, aEndianessRead);
      Write(aValue.w, aEndianessRead);
    }
    
    template <>
    void Write<glm::mat4>(glm::mat4 const& aValue, Endianess aEndianessRead)
    {
      Write(aValue[0], aEndianessRead);
      Write(aValue[1], aEndianessRead);
      Write(aValue[2], aEndianessRead);
      Write(aValue[3], aEndianessRead);
    }

    
    constexpr f32 cMATH_COLOR_CHAR = 255.0f;

    
	  glm::vec4 ReadABGR8()
    {
      glm::vec4 value;
		  value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
      return value;
	  }
    
    glm::vec4 ReadABGR8()
    {
      glm::vec4 value;
		  value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
      return value;
	  }
    
    glm::vec4 ReadRGBA8()
    {
      glm::vec4 value;
		  value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
		  value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
      return value;
	  }

	  void WriteRGBA8(glm::vec4 aValue) {
		  Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
	  }

	  void WriteABGR8(glm::vec4 aValue) {
		  Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
	  }

	  void WriteARGB8(glm::vec4 aValue) {
		  Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
		  Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
	  }

    glm::vec3 ReadNormal360(Endianess aEndianessRead)
    {
	    unsigned int value = Read<u32>(aEndianessRead);
      glm::vec3 toReturn;

	    toReturn.x = ((value&0x00000400 ? -1 : 0) + (float)((value>>2)&0x0FF)  / 256.0f);
	    toReturn.y = ((value&0x00200000 ? -1 : 0) + (float)((value>>13)&0x0FF) / 256.0f);
	    toReturn.z = ((value&0x80000000 ? -1 : 0) + (float)((value>>23)&0x0FF) / 256.0f);

      return toReturn;
    }
    
    glm::vec3 ReadNormal360()
    {
      return ReadNormal360(mFileDefaultEndianess);
    }

    
    template <typename tType>
    void Write(tType const& aValue)
    {
      Write(aValue, mFileDefaultEndianess);
    }

    void WriteByte(u8 aByte, size_t aNumber)
    {
      for (size_t i = 0; i < aNumber; ++i)
        WriteData(aByte);
    }

    void SetAddress(size_t aAddress)
    {
      fseek(mFile, aAddress + mGlobalOffset, SEEK_SET);
    }

    void OffsetAddress(size_t aOffset)
    {
      fseek(mFile, aOffset, SEEK_CUR);
    }

    // Goes to the end of the file.
    void GoToEnd()
    {
      fseek(mFile, 0, SEEK_END);
    }

    size_t GetCurrentAddress()
    {
			return ftell(mFile) - mGlobalOffset;
    }
    
    size_t File::GetFileSize()
    {
	    size_t currentAddress = GetCurrentAddress();
	    fseek(mFile, 0, SEEK_END);
	    size_t fileSize = ftell(mFile);
	    SetAddress(currentAddress);

	    return fileSize;
    }

    

	size_t File::FixPadding(size_t aMultiple)
	{
		size_t address = GetCurrentAddress();
		size_t padding = aMultiple - (address%aMultiple);

		if (padding == aMultiple)
			return 0;

    WriteByte(0, padding);

		return padding;
	}

	size_t File::FixPaddingRead(size_t aMultiple)
	{
		size_t address = GetCurrentAddress();
		size_t padding = aMultiple - (address%aMultiple);

		if (padding == aMultiple)
			return 0;

		SetAddress(address + padding);
		return padding;
	}

  void SetRootNodeAddress(size_t aRootNodeAddress)
  {
    mRootNodeAddress = aRootNodeAddress;
  }

  void SortAddressTable()
  {
    static_assert(false, "Implement");
  }

  std::list<size_t> GetAddressTable()
  {
    static_assert(false, "Implement");
  }
  
  void ReadAddressTableBBIN(u32 aSize)
  {
    static_assert(false, "Implement");
  }

  void WriteAddressTableBBIN()
  {
    static_assert(false, "Implement");
  }


  private:
    template <typename tType>
    tType ReadData()
    {
      tType value;
		  fread((void*)&value, sizeof(tType), 1, mFile);
      return value;
    }
  
    template <typename tType>
    void WriteData(tType& aValue)
    {
      tType valueToWrite = aValue;

      fwrite((void*)&valueToWrite, sizeof(tType), 1, mFile);
    }

    FILE* mFile = nullptr;
    Style mStyle;
    Endianess mFileDefaultEndianess;
    size_t mGlobalOffset = 0;
    size_t mRootNodeAddress = 0;
		std::list<size_t> mFinalAddressTable;
  };
}