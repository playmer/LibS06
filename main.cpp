#include "source/File.hpp"
#include "source/S06XnFile.h"

#include "SOIS/ImGuiSample.hpp"
#include "SOIS/ApplicationContext.hpp"

#include "nfd.h"

#include "imgui_memory_editor.h"

//int main()
//{
//  using namespace LibS06;
//
//  std::string fileName = "F:/Roms/Xbox360/Sonic the Hedgehog/Arcs/player_sonic/win32/player/sonic_new/sonic_Root.xno";
//
//  SonicXNFile file(fileName);
//
//  return 0;
//}

void ShowXnObjectInfo(LibS06::SonicXNObject const* aTexture)
{
  if (nullptr == aTexture)
    return;

  if (ImGui::TreeNode("SonicXNTexture"))
  {

    ImGui::TreePop();
  }
}

void ShowXnEffectsInfo(LibS06::SonicXNEffect const* aTexture)
{
  if (nullptr == aTexture)
    return;

  if (ImGui::TreeNode("SonicXNEffects"))
  {

    ImGui::TreePop();
  }
}

void ShowXnBonesInfo(LibS06::SonicXNBones const* aTexture)
{
  if (nullptr == aTexture)
    return;

  if (ImGui::TreeNode("SonicXNBones"))
  {

    ImGui::TreePop();
  }
}

void ShowXnMotionInfo(LibS06::SonicXNMotion const* aTexture)
{
  if (nullptr == aTexture)
    return;

  if (ImGui::TreeNode("SonicXNMotion"))
  {

    ImGui::TreePop();
  }
}

void ShowXnTextureInfo(LibS06::SonicXNTexture const* aTexture)
{
  if (nullptr == aTexture)
    return;

  if (ImGui::TreeNode("SonicXNTexture"))
  {

    ImGui::TreePop();
  }
}

void ShowXnFileInfo(LibS06::SonicXNFile* aFile)
{
  ShowXnBonesInfo(aFile->getBones());
  ShowXnTextureInfo(aFile->getTexture());
  ShowXnObjectInfo(aFile->getObject());
  ShowXnEffectsInfo(aFile->getEffect());
  ShowXnMotionInfo(aFile->getMotion());
}

bool HighlightBytes(const ImU8* aData, size_t aOffset, void* aUserData)
{
  LibS06::SonicXNFile* s06File = static_cast<LibS06::SonicXNFile*>(aUserData);

  return s06File->GetReadFile()->GetDataInfo()[aOffset];
}

int main(int, char**)
{
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  
  SOIS::ApplicationInitialization();
  SOIS::ApplicationContextConfig config;
  config.aBlocking = false;
  config.aWindowName = "LibSonic06_TestApp";
  
  SOIS::ApplicationContext context{config};
  SOIS::ImGuiSample sample;

  std::unique_ptr<LibS06::SonicXNFile> sonic06File;

  MemoryEditor memoryEditor;

  memoryEditor.HighlightFn = HighlightBytes;
  memoryEditor.HighlightColor = ImColor(204, 153, 0);
  
  while (context.Update())
  {
    if (ImGui::Begin("MainWindow"))
    {
      if (ImGui::Button("Open File"))
      {
        nfdchar_t* outPath;
        NFD_OpenDialog("xno", "", &outPath);
        
        std::string fileName = outPath;
        sonic06File = std::make_unique<LibS06::SonicXNFile>(fileName);
        memoryEditor.userData = sonic06File.get();

        free(outPath);
      }

      if (sonic06File)
      {
        ShowXnFileInfo(sonic06File.get());

        auto& data = sonic06File->GetReadFile()->GetData();
        
        memoryEditor.DrawWindow("Memory Editor", data.data(), data.size());
      }

      ImGui::End();
    }
  }
  
  return 0;
}
