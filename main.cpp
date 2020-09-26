
#include "File.hpp"

int main()
{
  using namespace LibS06;

  File file("", File::Style::Read, Endianess::Big);

  file.Read<u16>();
  return 0;
}