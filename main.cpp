
#include "source/File.hpp"
#include "source/S06XnFile.h"

int main()
{
  using namespace LibS06;

  std::string fileName = "F:/Roms/Xbox360/Sonic the Hedgehog/Arcs/player_sonic/win32/player/sonic_new/sonic_Root.xno";

  SonicXNFile file(fileName);

  return 0;
}