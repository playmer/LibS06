
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include "half.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/common.hpp"

#if defined(_MSC_VER) && !defined(__EDG__)
  #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace LibS06
{
  inline int gPositionToBreakOn = -1;
  inline int gAddressToBreakOn = -1;

  using i8 = char;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;

  using u8 = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  using u64 = std::uint64_t;

  using f32 = float;
  using f64 = double;

  enum class Endianess
  {
    Little = 0, 
    Big = 1
  };

  class File;

  struct TracingInfo
  {
    TracingInfo(char const* aName, size_t aStartInBytes, size_t aEndInBytes)
    {
      Name = aName;
      StartInBytes = aStartInBytes;
      EndInBytes = aEndInBytes;
    }

    std::string Name;
    size_t StartInBytes;
    size_t EndInBytes;
    size_t Depth;

    std::vector<std::unique_ptr<TracingInfo>> Children;

    void Place(std::unique_ptr<TracingInfo> aInfo);
    void Place(File* aFile);

    void Sort();

    void CalculateDepth(size_t aStart = 0);
    std::vector<TracingInfo*> Flatten();

    size_t Range()
    {
      return EndInBytes - StartInBytes;
    }
  
  private:
    void Flatten(std::vector<TracingInfo*>& aOut)
    {
      aOut.push_back(this);

      for (auto& child : Children)
        child->Flatten(aOut);
    }
  };

  inline bool IsBigEndian(void)
  {
      union {
          uint32_t i;
          char c[4];
      } bint = {0x01020304};

      return bint.c[0] == 1; 
  }

  inline Endianess SystemEndianess()
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

  std::vector<char> ReadFileIntoMemory(std::string aName);

  class File
  {
  public:
    struct AddressReadData
    {
      char const* File;
      char const* Function;
      size_t Line;
      size_t BytesRead;
    };

    enum class Style
    {
      Read,
      Write
    };

    File(std::string aFile, Style aStyle, Endianess aEndianess = Endianess::Little);

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
      return Read<tType>(mFileDefaultEndianess);
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
    
    void LogRead(const char* aFileName, size_t aLineInFile, const char* aFunctionName, size_t aBytesRead);
    void LogReadAddress(const char* aFileName, size_t aLineInFile, const char* aFunctionName, size_t aBytesRead);
    size_t ReadAddress(Endianess aEndianess, const char* aFileName, size_t aLineInFile, const char* aFunction);
    size_t ReadAddress(bool aBigEndian, const char* aFileName, size_t aLineInFile, const char* aFunction);
    size_t ReadAddressFileEndianess(const char* aFileName, size_t aLineInFile, const char* aFunction);
    void WriteAddress(size_t aAddress, Endianess aEndianess);
    void WriteAddressFileEndianess(size_t aAddress);
    void WriteNullTerminatedString(char const* aString);
    std::string ReadString(size_t aSize);
    std::string ReadNullTerminatedString();
    void ReadStream(void* aStream, size_t aSize);
    void WriteStream(void const* aStream, size_t aSize);
    
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

    
    const f32 cMATH_COLOR_CHAR = 255.0f;

    
    glm::vec4 ReadARGB8();
    glm::vec4 ReadABGR8();
    glm::vec4 ReadRGBA8();
    void WriteRGBA8(glm::vec4 aValue);
    void WriteABGR8(glm::vec4 aValue);
    void WriteARGB8(glm::vec4 aValue);
    glm::vec3 ReadNormal360(Endianess aEndianessRead);
    glm::vec3 ReadNormal360();
    
    template <typename tType>
    void Write(tType const& aValue)
    {
      Write(aValue, mFileDefaultEndianess);
    }

    void WriteByte(u8 aByte, size_t aNumber);
    void SetAddress(size_t aAddress);
    void OffsetAddress(size_t aOffset);

      // Goes to the end of the file.
    void GoToEnd();
    size_t GetCurrentAddress();
    size_t GetFileSize();

    size_t FixPadding(size_t aMultiple);
    size_t FixPaddingRead(size_t aMultiple);
    void SetRootNodeAddress(size_t aRootNodeAddress);
    void SortAddressTable();
    std::vector<size_t> const& GetAddressTable();
    void ReadAddressTableBBIN(u32 aTableSize);
    void WriteAddressTableBBIN(size_t negativeOffset = 0);

    std::map<size_t, AddressReadData> const& GetAddressMap();
    size_t GetRootNodeAddress();
    
    std::vector<char>& GetData()
    {
      return Data;
    }

    std::vector<bool> const& GetDataInfo() const
    {
      return DataInfo;
    }

    void AddLabel(char const* aName, size_t aStartInBytes, size_t aEndInBytes)
    {
      mTracingInfos.emplace_back(std::make_unique<TracingInfo>(aName, aStartInBytes, aEndInBytes));
    }

    std::vector<std::unique_ptr<TracingInfo>>& GetTracingInfos()
    {
      return mTracingInfos;
    }

    size_t GetGlobalOffset()
    {
      return mGlobalOffset;
    }

  private:
    template <typename tType>
    tType ReadData()
    {
      tType value;
      tType value2;

      fread((void*)&value, sizeof(tType), 1, mFile);
      memcpy((void*)&value2, Data.data() + CurrentPosition, sizeof(tType));
      
      // Mark that we've read these bytes
      std::fill_n(DataInfo.begin() + CurrentPosition, sizeof(tType), true);

      assert(value == value2);

      if ((-1 != gPositionToBreakOn) && (CurrentPosition == gPositionToBreakOn)) __debugbreak();

      CurrentPosition += sizeof(tType);

      return value;
    }
  
    template <typename tType>
    void WriteData(tType& aValue)
    {
      tType valueToWrite = aValue;

      fwrite((void*)&valueToWrite, sizeof(tType), 1, mFile);
    }

    Endianess mFileDefaultEndianess;
    size_t mGlobalOffset = 0;
    size_t mRootNodeAddress = 0;
    std::vector<size_t> mFinalAddressTable;

    std::vector<std::unique_ptr<TracingInfo>> mTracingInfos;
    std::map<size_t, AddressReadData> mAddressReadMap;
    std::map<size_t, AddressReadData> mReadMap;
    void GetRangesRead();

    
    Style mStyle;

    // Reading
    std::vector<char> Data;
    std::vector<bool> DataInfo;
    size_t CurrentPosition = 0;
    bool mHasRootNodeAddressBeenSet = false;
    
    // Writing
    FILE* mFile = nullptr;
  };
}