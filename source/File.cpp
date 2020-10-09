#include "File.hpp"

namespace LibS06
{
  std::vector<char> ReadFileIntoMemory(std::string aName)
  {
    FILE *f = fopen(aName.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<char> data;
    data.resize(fsize);
    fread(data.data(), 1, fsize, f);
    fclose(f);

    return data;
  }

  File::File(std::string aFile, Style aStyle, Endianess aEndianess /*= Endianess::Little*/)
    : mStyle{aStyle}
    , mFileDefaultEndianess{aEndianess}
  {
    if (aStyle == Style::Read)
    {
      Data = ReadFileIntoMemory(aFile);
      DataInfo.resize(Data.size(), false);
    }

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

  void File::LogReadAddress(const char* aFileName, size_t aLineInFile, const char* aFunctionName)
  {
    std::string line;
    line += aFileName;
    line += "(";
    line += std::to_string(aLineInFile);
    line += "): Address read in ";
    line += aFunctionName;
    line += " here.";
      
    mAddressReadMap.emplace(GetCurrentAddress() - mRootNodeAddress, line);
  }

  size_t File::ReadAddress(Endianess aEndianess, const char* aFileName, size_t aLineInFile, const char* aFunction)
  {
    LogReadAddress(aFileName, aLineInFile, aFunction);

    return Read<u32>(aEndianess) + mRootNodeAddress;
  }

  size_t File::ReadAddress(bool aBigEndian, const char* aFileName, size_t aLineInFile, const char* aFunction)
  {
    return ReadAddress(aBigEndian ? Endianess::Big : Endianess::Little, aFileName, aLineInFile, aFunction);
  }
    
  size_t File::ReadAddressFileEndianess(const char* aFileName, size_t aLineInFile, const char* aFunction)
  {
    return ReadAddress(mFileDefaultEndianess, aFileName, aLineInFile, aFunction);
  }

  void File::WriteAddress(size_t aAddress, Endianess aEndianess)
  {
    if (aEndianess == Endianess::Big) 
      mFinalAddressTable.push_back(GetCurrentAddress() - mRootNodeAddress);

    // Addresses are written as an offset into the file from the mRootNodeAddress.
    Write<u32>(aAddress - mRootNodeAddress);
  }
    
  void File::WriteAddressFileEndianess(size_t aAddress)
  {
    WriteAddress(aAddress, mFileDefaultEndianess);
  }
    
  void File::WriteNullTerminatedString(char const* aString)
  {
    for (; '\0' != *aString; ++aString)
      Write<char>(*aString, mFileDefaultEndianess);

    Write<char>('\0', mFileDefaultEndianess);
  }

  std::string File::ReadString(size_t aSize)
  {
    std::string value;

    for (size_t i = 0; i < aSize; ++i)
      value += Read<char>();

    return value;
  }

  std::string File::ReadNullTerminatedString()
  {
    std::string value;
    auto character = Read<char>();

    while ('\0' != character)
    {
      value += character;
      character = Read<char>();
    } 

    return value;
  }

  void File::ReadStream(void* aStream, size_t aSize)
  {
    fread(aStream, 1, aSize, mFile);
    auto result = memcmp(aStream, Data.data() + CurrentPosition, aSize);
    
    // Mark that we've read these bytes
    std::fill_n(DataInfo.begin() + CurrentPosition, aSize, true);

    assert(0 == result);

    CurrentPosition += aSize;
  }
    
  void File::WriteStream(void const* aStream, size_t aSize)
  {
    fwrite(aStream, 1, aSize, mFile);
  }
    
  glm::vec4 File::ReadARGB8()
  {
    glm::vec4 value;
    value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    return value;
  }
    
  glm::vec4 File::ReadABGR8()
  {
    glm::vec4 value;
    value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    return value;
  }
    
  glm::vec4 File::ReadRGBA8()
  {
    glm::vec4 value;
    value.r = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.g = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.b = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    value.a = ((float)Read<u8>()) / cMATH_COLOR_CHAR;
    return value;
  }

  void File::WriteRGBA8(glm::vec4 aValue) {
    Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
  }

  void File::WriteABGR8(glm::vec4 aValue) {
    Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
  }

  void File::WriteARGB8(glm::vec4 aValue) {
    Write<u8>((unsigned char)(aValue.a * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.r * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.g * cMATH_COLOR_CHAR));
    Write<u8>((unsigned char)(aValue.b * cMATH_COLOR_CHAR));
  }

  glm::vec3 File::ReadNormal360(Endianess aEndianessRead)
  {
    unsigned int value = Read<u32>(aEndianessRead);
    glm::vec3 toReturn;

    toReturn.x = ((value&0x00000400 ? -1 : 0) + (float)((value>>2)&0x0FF)  / 256.0f);
    toReturn.y = ((value&0x00200000 ? -1 : 0) + (float)((value>>13)&0x0FF) / 256.0f);
    toReturn.z = ((value&0x80000000 ? -1 : 0) + (float)((value>>23)&0x0FF) / 256.0f);

    return toReturn;
  }
    
  glm::vec3 File::ReadNormal360()
  {
    return ReadNormal360(mFileDefaultEndianess);
  }

  void File::WriteByte(u8 aByte, size_t aNumber)
  {
    for (size_t i = 0; i < aNumber; ++i)
      WriteData(aByte);
  }

  void File::SetAddress(size_t aAddress)
  {
    fseek(mFile, aAddress + mGlobalOffset, SEEK_SET);
    CurrentPosition = aAddress + mGlobalOffset;
  }

  void File::OffsetAddress(size_t aOffset)
  {
    fseek(mFile, aOffset, SEEK_CUR);
    CurrentPosition += aOffset;
  }

  // Goes to the end of the file.
  void File::GoToEnd()
  {
    fseek(mFile, 0, SEEK_END);
  }

  size_t File::GetCurrentAddress()
  {
    size_t currentAddress = ftell(mFile) - mGlobalOffset;
    assert(currentAddress == CurrentPosition);
    return currentAddress;
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

  void File::SetRootNodeAddress(size_t aRootNodeAddress)
  {
    mRootNodeAddress = aRootNodeAddress;
  }

  void File::SortAddressTable()
  {
    std::sort(mFinalAddressTable.begin(), mFinalAddressTable.end());
  }

  std::vector<size_t> const& File::GetAddressTable()
  {
    return mFinalAddressTable;
  }
  
  void File::ReadAddressTableBBIN(u32 aTableSize)
  {
    size_t current_address = mRootNodeAddress;

    std::vector<unsigned char> offsetTable;
    offsetTable.resize(aTableSize);
    
    ReadStream((void*)offsetTable.data(), aTableSize);

    mFinalAddressTable.clear();

    for (size_t i=0; i < aTableSize; i++)
    {
      size_t low = offsetTable[i] & 0x3F;

      if ((offsetTable[i] & 0x80) && (offsetTable[i] & 0x40))
      {
        i += 3;
        current_address += (low * 0x4000000) + (offsetTable[i-2] * 0x40000) + (offsetTable[i-1] * 0x400) + (offsetTable[i] * 0x4);
      }
      else if (offsetTable[i] & 0x80)
      {
        i++;
        current_address += (low * 0x400) + (offsetTable[i] * 4);
      }
      else if (offsetTable[i] & 0x40)
      {
        current_address += 4 * low;
      }

      mFinalAddressTable.push_back(current_address - mRootNodeAddress);
    }
  }

  void File::WriteAddressTableBBIN(size_t negativeOffset /* = 0*/)
  {
    size_t current_address = negativeOffset;
    for (auto it = mFinalAddressTable.begin(); it != mFinalAddressTable.end(); it++)
    {
      size_t difference = (*it) - current_address;

      if (difference > 0xFFFC)
      {
        unsigned int offset_int = 0xC0000000 | (difference >> 2);
        Write<u32>(offset_int, Endianess::Big);
      }
      else if (difference > 0xFC)
      {
        unsigned short offset_short = 0x8000 | (difference >> 2);
        Write<u16>(offset_short, Endianess::Big);
      }
      else
      {
        char offset_byte = 0x40 | (difference >> 2);
        Write<char>(offset_byte);
      }

      current_address += difference;
    }
  }
  
  std::map<size_t, std::string> const& File::GetAddressMap()
  {
    return mAddressReadMap;
  }

  size_t File::GetRootNodeAddress()
  {
    return mRootNodeAddress;
  }
}