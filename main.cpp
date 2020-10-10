#include <memory>
#include <vector>

#include "source/File.hpp"
#include "source/S06XnFile.h"

#include "SOIS/ImGuiSample.hpp"
#include "SOIS/ApplicationContext.hpp"

#include "nfd.h"

#include "imgui_memory_editor.h"

#include "imgui_widget_flamegraph.h"
#include "imgui_stdlib.h"



struct TracingInfo
{
  TracingInfo(char const* aName, size_t aStartInBytes, size_t aEndInBytes)
  {
    Name = aName;
    StartInBytes = aStartInBytes;
    EndInBytes = aEndInBytes;
  }
  
  TracingInfo(char const* aName, LibS06::SonicXNSection* aSection)
  {
    Name = aName;
    StartInBytes = aSection->getAddress();
    EndInBytes = aSection->getAddress() + aSection->getSectionSize();
  }

  char const* Name;
  size_t StartInBytes;
  size_t EndInBytes;
  size_t Depth;

  std::vector<std::unique_ptr<TracingInfo>> Children;

  void Sort()
  {
    std::sort(Children.begin(), Children.end(), [](std::unique_ptr<TracingInfo> const& left, std::unique_ptr<TracingInfo> const& right)
    {
      return left->StartInBytes < right->StartInBytes;
    });

    for (auto& child : Children)
      Sort();
  }

  void CalculateDepth(size_t aStart = 0)
  {
    Depth = aStart;
    
    for (auto& child : Children)
      child->CalculateDepth(Depth + 1);
  }

  std::vector<TracingInfo*> Flatten()
  {
    std::vector<TracingInfo*> toReturn;
    toReturn.push_back(this);

    for (auto& child : Children)
      child->Flatten(toReturn);

    return toReturn;
  }

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
























std::unique_ptr<TracingInfo> GetTracingInfo(LibS06::SonicXNFile* aFile)
{
  std::unique_ptr<TracingInfo> root = std::make_unique<TracingInfo>("Whole File", 0, aFile->GetReadFile()->GetData().size());

  root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNInfo", aFile->info->getAddress(), aFile->info->getAddress() + aFile->info->getSectionSize()));

  for (auto& unknownSection : aFile->sections)
  {
    LibS06::SonicXNSection* section = nullptr;
    char const* sectionName = nullptr;
    if (unknownSection->getHeader() == aFile->header_texture) {
      section = static_cast<LibS06::SonicXNTexture*>(unknownSection);
      root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNTexture", section));
    }
    else if (unknownSection->getHeader() == aFile->header_effect) {
      section = static_cast<LibS06::SonicXNEffect*>(unknownSection);
      root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNEffect", section));
    }
    else if (unknownSection->getHeader() == aFile->header_bones) {
      section = static_cast<LibS06::SonicXNBones*>(unknownSection);
      root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNBones", section));
    }
    else if (unknownSection->getHeader() == aFile->header_object) {
      section = static_cast<LibS06::SonicXNObject*>(unknownSection);
      root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNObject", section));
    }
    else if (unknownSection->getHeader() == aFile->header_motion) {
      section = static_cast<LibS06::SonicXNMotion*>(unknownSection);
      root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNMotion", section));
    }
  }

  root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNOffsetTable", aFile->offset_table));
  root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNFooter", aFile->footer));
  root->Children.emplace_back(std::make_unique<TracingInfo>("SonicXNEnd", aFile->end));

  return root;
}





void TracingValuesGetter(float* start, float* end, ImU8* level, const char** caption, const void* data, int idx)
{
  auto info = *(reinterpret_cast<TracingInfo*const*>(data) + idx);
  if (start)
  {
    *start = (float)info->StartInBytes;
  }
  if (end)
  {
    *end = (float)info->EndInBytes;
  }
  if (level)
  {
    *level = (ImU8)info->Depth;
  }
  if (caption)
  {
    *caption = info->Name;
  }
}

void ShowTracingInfo(TracingInfo* aTracingInfoRoot, std::vector<TracingInfo*>& aTracingInfosFlattened, std::string& fileName)
{
  static TracingInfo* tracingInfoRootLast = nullptr;
  static double zoom = 1.0f;
  const double zoomSpeed = 1.15f;

  if (tracingInfoRootLast != aTracingInfoRoot)
  {
    zoom = ImGui::GetWindowSize().x / aTracingInfoRoot->EndInBytes;
    tracingInfoRootLast = aTracingInfoRoot;
  }

  double lastZoom = zoom;

  float wheel = ImGui::GetIO().MouseWheel;
  if (0.0f > wheel)
  {
    zoom /= zoomSpeed;
  }
  else if (0.0f < wheel)
  {
    zoom *= zoomSpeed;
  }

  ImGui::Begin("File Layout");

  ImGui::BeginChild("scrolling", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
  
  {
    static float lastScrollMax = ImGui::GetScrollMaxX();
    float currentScrollMax = ImGui::GetScrollMaxX();
    static float lastScroll = ImGui::GetScrollX();
    float currentScroll = ImGui::GetScrollX();
    float ratio = lastScroll / lastScrollMax;
    float scroll = ratio * currentScrollMax;

    if (lastScrollMax != currentScrollMax)
    {
      ImGui::SetScrollX(scroll);
      currentScroll = scroll;
    }
  
    lastScrollMax = currentScrollMax;
    lastScroll = currentScroll;
  }
  
  ImGuiWidgetFlameGraph::PlotFlame(
    "Sonic 06 File", 
    TracingValuesGetter, 
    aTracingInfosFlattened.data(), 
    aTracingInfosFlattened.size(), 
    0, 
    fileName.c_str(), 
    0.f,
    aTracingInfoRoot->EndInBytes,
    ImVec2(aTracingInfoRoot->EndInBytes * zoom, 0.0f));
  

  if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
  {
    ImVec2 offset(0.0f, 0.0f);

    offset.x -= ImGui::GetIO().MouseDelta.x;
    offset.y -= ImGui::GetIO().MouseDelta.y;

    ImGui::SetScrollX(ImGui::GetScrollX() + offset.x);
    ImGui::SetScrollY(ImGui::GetScrollY() + offset.y);
  }

  ImGui::EndChild();
  ImGui::End();
}





















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

  std::unique_ptr<TracingInfo> tracingInfoRoot;
  std::vector<TracingInfo*> tracingInfosFlattened;

  std::string fileName = "F:/Roms/Xbox360/Sonic the Hedgehog/Arcs/player_sonic/win32/player/sonic_new/so_itm_sbungle_L.xno";
  
  while (context.Update())
  {
    if (ImGui::Begin("MainWindow"))
    {
      ImGui::InputText("File", &fileName);
      ImGui::InputInt("PositionToBreakOn", &LibS06::gPositionToBreakOn);
      ImGui::InputInt("AddressToBreakOn", &LibS06::gAddressToBreakOn);

      bool open = ImGui::Button("Open File");
      bool reload = fileName.size() != 0 ? ImGui::Button("Reload File") : false;

      if (open)
      {
        nfdchar_t* outPath;
        NFD_OpenDialog("xno", "", &outPath);
        fileName = outPath;
        free(outPath);
      }

      if (open || reload)
      {
        sonic06File = std::make_unique<LibS06::SonicXNFile>(fileName);
        memoryEditor.userData = sonic06File.get();
        tracingInfoRoot = GetTracingInfo(sonic06File.get());
        tracingInfoRoot->CalculateDepth();
        tracingInfosFlattened = tracingInfoRoot->Flatten();
      }

      if (sonic06File)
      {
        ShowXnFileInfo(sonic06File.get());

        auto& data = sonic06File->GetReadFile()->GetData();
        
        memoryEditor.DrawWindow("Memory Editor", data.data(), data.size());

        ShowTracingInfo(tracingInfoRoot.get(), tracingInfosFlattened, fileName);
      }

      ImGui::End();
    }
  }
  
  return 0;
}
