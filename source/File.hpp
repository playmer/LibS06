
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <type_traits>

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
      : cNeedEndianessSwap{aEndianess != SystemEndianess()}
      , mStyle{aStyle}
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
    }

    template <typename tType>
    tType Read()
    {
      tType value; // = ...
    
		  fread((void*)&value, sizeof(tType), 1, mFile);

      if (cNeedEndianessSwap)
        SwapEndian(value);
      return value;
    }
  
    template <typename tType>
    void Read(tType& aValue)
    {
      aValue = ReadData<tType>();
      
      if (cNeedEndianessSwap)
        SwapEndian(valueToWrite);
    }
  
    template <typename tType>
    void Write(tType& aValue)
    {
      tType valueToWrite = aValue;
    
      if (cNeedEndianessSwap)
        SwapEndian(valueToWrite);

      WriteData<tType>(valueToWrite);
    }

    void SetAddress(size_t aAddress)
    {
      fseek(mFile, aAddress, SEEK_SET);
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
    const bool cNeedEndianessSwap;
  };

}