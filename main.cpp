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


struct FileTracingContext
{
  MemoryEditor memoryEditor;
  std::unique_ptr<LibS06::TracingInfo> tracingInfoRoot;
  std::vector<LibS06::TracingInfo*> tracingInfosFlattened;
  std::string fileName = "F:/Roms/Xbox360/Sonic the Hedgehog/Arcs/player_sonic/win32/player/sonic_new/so_itm_sbungle_L.xno";
};













std::unique_ptr<LibS06::TracingInfo> FromSection(char const* aName, LibS06::SonicXNSection* aSection)
{
  return std::make_unique<LibS06::TracingInfo>(
    aName,
    aSection->getAddress(),
    aSection->getAddress() + aSection->getSectionSize());
}


std::unique_ptr<LibS06::TracingInfo> GetTracingInfo(LibS06::SonicXNFile* aFile)
{
  std::unique_ptr<LibS06::TracingInfo> root = std::make_unique<LibS06::TracingInfo>("Whole File", 0, aFile->GetReadFile()->GetData().size());

  root->Children.emplace_back(FromSection("SonicXNInfo", aFile->info));

  for (auto& unknownSection : aFile->sections)
  {
    LibS06::SonicXNSection* section = nullptr;
    char const* sectionName = nullptr;
    if (unknownSection->getHeader() == aFile->header_texture) {
      section = static_cast<LibS06::SonicXNTexture*>(unknownSection);
      root->Children.emplace_back(FromSection("SonicXNTexture", section));
    }
    else if (unknownSection->getHeader() == aFile->header_effect) {
      section = static_cast<LibS06::SonicXNEffect*>(unknownSection);
      root->Children.emplace_back(FromSection("SonicXNEffect", section));
    }
    else if (unknownSection->getHeader() == aFile->header_bones) {
      section = static_cast<LibS06::SonicXNBones*>(unknownSection);
      root->Children.emplace_back(FromSection("SonicXNBones", section));
    }
    else if (unknownSection->getHeader() == aFile->header_object) {
      section = static_cast<LibS06::SonicXNObject*>(unknownSection);
      root->Children.emplace_back(FromSection("SonicXNObject", section));
    }
    else if (unknownSection->getHeader() == aFile->header_motion) {
      section = static_cast<LibS06::SonicXNMotion*>(unknownSection);
      root->Children.emplace_back(FromSection("SonicXNMotion", section));
    }
  }

  root->Children.emplace_back(FromSection("SonicXNOffsetTable", aFile->offset_table));
  root->Children.emplace_back(FromSection("SonicXNFooter", aFile->footer));
  root->Children.emplace_back(FromSection("SonicXNEnd", aFile->end));

  root->Place(aFile->GetReadFile());
  root->Sort();
  root->CalculateDepth();

  return root;
}



void TracingValueClicked(void* data, int idx)
{
  auto context = reinterpret_cast<FileTracingContext*>(data);
  auto info = context->tracingInfosFlattened[idx];

  context->memoryEditor.GotoAddrAndHighlight(info->StartInBytes, info->EndInBytes);
}

void TracingValuesGetter(float* start, float* end, ImU8* level, const char** caption, const void* data, int idx)
{
  auto context = reinterpret_cast<FileTracingContext const*>(data);
  auto info = context->tracingInfosFlattened[idx];

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
    *caption = info->Name.c_str();
  }
}

void ShowTracingInfo(FileTracingContext& aContext)
{
  static LibS06::TracingInfo* tracingInfoRootLast = nullptr;
  static double zoom = 1.0f;
  const double zoomSpeed = 1.15f;

  if (tracingInfoRootLast != aContext.tracingInfoRoot.get())
  {
    zoom = ImGui::GetWindowSize().x / aContext.tracingInfoRoot->EndInBytes;
    tracingInfoRootLast = aContext.tracingInfoRoot.get();
  }

  double lastZoom = zoom;
  float wheel = ImGui::GetIO().MouseWheel;

  ImGui::Begin("File Layout", nullptr, ImGuiWindowFlags_NoCollapse);

  ImGui::BeginChild("scrolling", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);

  if (ImGui::IsWindowHovered())
  {
    if (0.0f > wheel)
    {
      zoom /= zoomSpeed;
    }
    else if (0.0f < wheel)
    {
      zoom *= zoomSpeed;
    }
  }
  
  {
    static float lastScrollMax = ImGui::GetScrollMaxX();
    static float lastScroll = ImGui::GetScrollX();
    const float currentScrollMax = ImGui::GetScrollMaxX();
    const float currentScroll = ImGui::GetScrollX();
    const float ratio = lastScroll / lastScrollMax;
    const float scroll = ratio * currentScrollMax;

    if (lastScrollMax != currentScrollMax)
    {
      ImGui::SetScrollX(scroll);
      lastScroll = scroll;
    }
    else
    {
      lastScroll = currentScroll;
    }
  
    lastScrollMax = currentScrollMax;
  }
  
  ImGuiWidgetFlameGraph::PlotFlame(
    "Sonic 06 File", 
    TracingValuesGetter, 
    &aContext,
    aContext.tracingInfosFlattened.size(),
    0, 
    aContext.fileName.c_str(), 
    0.f,
    aContext.tracingInfoRoot->EndInBytes,
    ImVec2(aContext.tracingInfoRoot->EndInBytes * zoom, 0.0f),
    TracingValueClicked);
  

  if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
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
  FileTracingContext fileContext;

  fileContext.memoryEditor.HighlightFn = HighlightBytes;
  fileContext.memoryEditor.HighlightColor = ImColor(204, 153, 0);

  
  while (context.Update())
  {
    ImGui::DockSpaceOverViewport();

    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse))
    {
      ImGui::InputText("File", &fileContext.fileName);
      ImGui::InputInt("PositionToBreakOn", &LibS06::gPositionToBreakOn);
      ImGui::InputInt("AddressToBreakOn", &LibS06::gAddressToBreakOn);

      bool open = ImGui::Button("Open File");
      bool reload = fileContext.fileName.size() != 0 ? ImGui::Button("Reload File") : false;

      if (open)
      {
        nfdchar_t* outPath;
        NFD_OpenDialog("xno", "", &outPath);
        fileContext.fileName = outPath;
        free(outPath);
      }

      if (open || reload)
      {
        sonic06File = std::make_unique<LibS06::SonicXNFile>(fileContext.fileName);
        fileContext.memoryEditor.userData = sonic06File.get();
        fileContext.tracingInfoRoot = GetTracingInfo(sonic06File.get());
        fileContext.tracingInfosFlattened = fileContext.tracingInfoRoot->Flatten();
      }

      if (sonic06File)
      {
        ShowXnFileInfo(sonic06File.get());

        auto& data = sonic06File->GetReadFile()->GetData();
        
        fileContext.memoryEditor.DrawWindow("Memory Editor", data.data(), data.size());

        ShowTracingInfo(fileContext);
      }

      ImGui::End();
    }
  }
  
  return 0;
}
