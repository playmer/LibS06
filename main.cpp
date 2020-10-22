#include <any>
#include <memory>
#include <vector>

#include <glm/gtx/quaternion.hpp>

#include "source/File.hpp"
#include "source/S06XnFile.h"

#include "SOIS/ImGuiSample.hpp"
#include "SOIS/ApplicationContext.hpp"

#include "nfd.h"

#include "imgui_memory_editor.h"

#include "imgui_widget_flamegraph.h"
#include "imgui_stdlib.h"

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

// Dimension struct is used for bounding box of 3D mesh
struct Dimension
{
  glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());
  glm::vec3 mMax = glm::vec3(-std::numeric_limits<float>::max());

  float GetRadius()
  {
    return std::max(glm::length(mMin), glm::length(mMax));
  }

  glm::vec3 GetCenter()
  {
    auto difference = mMax - mMin;

    return mMin + (difference * .5f);
  }
};

Dimension CalculateDimension(std::vector<glm::vec3>& verts)
{
  Dimension dimension;
  for (auto const& position : verts)
  {
    dimension.mMax.x = fmax(position.x, dimension.mMax.x);
    dimension.mMax.y = fmax(position.y, dimension.mMax.y);
    dimension.mMax.z = fmax(position.z, dimension.mMax.z);

    dimension.mMin.x = fmin(position.x, dimension.mMin.x);
    dimension.mMin.y = fmin(position.y, dimension.mMin.y);
    dimension.mMin.z = fmin(position.z, dimension.mMin.z);
  }

  return dimension;
}
struct OpenGLData
{
  ~OpenGLData()
  {
    gl::glDeleteShader(shaderProgram);
    gl::glDeleteBuffers(1, &VBO);
    gl::glDeleteBuffers(1, &EBO);
    gl::glDeleteVertexArrays(1, &VAO);
    gl::glDeleteTextures(1, &RenderedTexture);
    gl::glDeleteFramebuffers(1, &FrameBufferName);
  }
  
  size_t Vertices;
  size_t Indices;
  Dimension dimension;
  glm::vec3 cameraPosition{0,0,-20};
  float objectRotation = 0.f;
  int shaderProgram;
  unsigned int VBO, VAO, EBO;
  unsigned int FrameBufferName;
  unsigned int RenderedTexture;
  unsigned int depthrenderbuffer;
};

std::vector<unsigned int> GetIndices(std::vector<unsigned int> indices)
{
  std::vector<unsigned int> out;
	size_t additional_index=0;
	unsigned short last_index_1=0;
	unsigned short last_index_2=0;
	unsigned short index=0;
	int count=0;
		
	for (auto newIndex : indices) {
		last_index_1 = last_index_2;
		last_index_2 = index;
		
		index = newIndex;
		count++;
		
		//if ((index == last_index_1) || (index == last_index_2) || (last_index_1 == last_index_2)) {
		//	Error::AddMessage(Error::LogType::WARNING, "Invalid triangle found at " + ToString(file->GetCurrentAddress()-6) + ". Strip of size " + ToString(strip_sizes[m]) + " starts at " + ToString(index_address + additional_index*2));
		//	continue;
		//}
			
		if (count >= 3) {
			if (count%2==1) {
				out.push_back(last_index_1);
				out.push_back(last_index_2);
				out.push_back(last_index_1);
			}
			else {
				out.push_back(index);
				out.push_back(last_index_2);
				out.push_back(last_index_1);
			}
		}
		
		if (index == (unsigned short)0xFFFF) {
			printf("Unhandled case! Index with value 0xFFFF exists.\n");
		}
	}

  return out;
}

std::unique_ptr<OpenGLData> SetUpOpenGLData(LibS06::SonicXNFile* aFile)
{
  auto object = aFile->getObject();
  std::vector<glm::vec3> vertices;




  std::vector<unsigned int> tempIndices;

  for (auto vertexTable : object->vertex_tables)
  {
    for (auto vertex : vertexTable->vertices)
    {
      vertices.push_back(vertex->position);
    }
  }
  
  for (auto indexTable : object->index_tables)
  {
    tempIndices.insert(tempIndices.end(), indexTable->indices.begin(), indexTable->indices.end());
  }
  
  //std::vector<unsigned int> indices = GetIndices(tempIndices);
  std::vector<unsigned int> indices = tempIndices;

  auto data = std::make_unique<OpenGLData>();

  data->dimension = CalculateDimension(vertices);

  // The following is the Hello-Triangle example from learnopengl.com
  const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec4 outPosition;\n"
    "void main()\n"
    "{\n"
    "   outPosition = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
  const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 outPosition;\n"
    "void main()\n"
    "{\n"
    "   FragColor = outPosition;\n"
    //"   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";

  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  int vertexShader = gl::glCreateShader(gl::GL_VERTEX_SHADER);
  gl::glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  gl::glCompileShader(vertexShader);
  // check for shader compile errors
  int success;
  char infoLog[512];
  gl::glGetShaderiv(vertexShader, gl::GL_COMPILE_STATUS, &success);
  if (!success)
  {
    gl::glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // fragment shader
  int fragmentShader = gl::glCreateShader(gl::GL_FRAGMENT_SHADER);
  gl::glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  gl::glCompileShader(fragmentShader);
  // check for shader compile errors
  gl::glGetShaderiv(fragmentShader, gl::GL_COMPILE_STATUS, &success);
  if (!success)
  {
    gl::glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // link shaders
  data->shaderProgram = gl::glCreateProgram();
  gl::glAttachShader(data->shaderProgram, vertexShader);
  gl::glAttachShader(data->shaderProgram, fragmentShader);
  gl::glLinkProgram(data->shaderProgram);
  // check for linking errors
  gl::glGetProgramiv(data->shaderProgram, gl::GL_LINK_STATUS, &success);
  if (!success)
  {
    gl::glGetProgramInfoLog(data->shaderProgram, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  gl::glDeleteShader(vertexShader);
  gl::glDeleteShader(fragmentShader);
  
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  //float vertices[] = {
  //     0.5f,  0.5f, 0.0f,  // top right
  //     0.5f, -0.5f, 0.0f,  // bottom right
  //    -0.5f, -0.5f, 0.0f,  // bottom left
  //    -0.5f,  0.5f, 0.0f   // top left 
  //};
  data->Vertices = std::size(vertices);

  //unsigned int indices[] = {  // note that we start from 0!
  //    0, 1, 3,  // first Triangle
  //    1, 2, 3   // second Triangle
  //};
  data->Indices = std::size(indices);

  gl::glGenVertexArrays(1, &data->VAO);
  gl::glGenBuffers(1, &data->VBO);
  gl::glGenBuffers(1, &data->EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), 
  // and then configure vertex attributes(s).
  gl::glBindVertexArray(data->VAO);

  gl::glBindBuffer(gl::GL_ARRAY_BUFFER, data->VBO);
  gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), gl::GL_STATIC_DRAW);

  gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, data->EBO);
  gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), gl::GL_STATIC_DRAW);

  gl::glVertexAttribPointer(0, 3, gl::GL_FLOAT, gl::GL_FALSE, 3 * sizeof(float), (void*)0);
  gl::glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered
  // VBO as the vertex attribute's bound vertex buffer object so afterwards 
  // we can safely unbind
  gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

  // remember: do NOT unbind the EBO while a VAO is active as the bound 
  // element buffer object IS stored in the VAO; keep the EBO bound.
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires
  // a call to glBindVertexArray anyways so we generally don't unbind VAOs
  // (nor VBOs) when it's not directly necessary.
  gl::glBindVertexArray(0);

  ////////////////////////////////////////////////////////
  // Render to texture
  ////////////////////////////////////////////////////////
  
  ////////////////////////////////////////////////////////
  // Frame Buffer
  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  gl::glGenFramebuffers(1, &data->FrameBufferName);
  gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, data->FrameBufferName);

  // The texture we're going to render to
  gl::glGenTextures(1, &data->RenderedTexture);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  gl::glBindTexture(gl::GL_TEXTURE_2D, data->RenderedTexture);

  // Give an empty image to OpenGL ( the last "0" )
  gl::glTexImage2D(gl::GL_TEXTURE_2D, 0, gl::GL_RGBA, 800, 800, 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, 0);

  // Poor filtering. Needed !
  gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
  gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);

  ////////////////////////////////////////////////////////
  // Depth Buffer
  gl::glGenRenderbuffers(1, &data->depthrenderbuffer);
  gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, data->depthrenderbuffer);
  gl::glRenderbufferStorage(gl::GL_RENDERBUFFER, gl::GL_DEPTH_COMPONENT, 800, 800);
  gl::glFramebufferRenderbuffer(gl::GL_FRAMEBUFFER, gl::GL_DEPTH_ATTACHMENT, gl::GL_RENDERBUFFER, data->depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
  gl::glFramebufferTexture(gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0, data->RenderedTexture, 0);

  // Set the list of draw buffers.
  std::array<gl::GLenum, 1> DrawBuffers = {gl::GL_COLOR_ATTACHMENT0};
  gl::glDrawBuffers(1, DrawBuffers.data()); // "1" is the size of DrawBuffers
  // Always check that our framebuffer is ok
  if(gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) != gl::GL_FRAMEBUFFER_COMPLETE)
    return nullptr;

  return std::move(data);
}


void DrawOpenGLData(std::unique_ptr<OpenGLData>& aGLData)
{
  //gl::glDisable(gl::GL_CULL_FACE);
  
  ///////////////////////////////////////////////////////////////////////////
  // The rest of the loop is just the stuff you need to do to actually run
  // the hello-triangle example we set up above.
  ///////////////////////////////////////////////////////////////////////////
  // Render to our framebuffer
  gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, aGLData->FrameBufferName);
  gl::glViewport(0,0,800,800); // Render on the whole framebuffer, complete from the lower left corner to the upper right
  
  gl::glClearColor(.45f, .45f, .45f, 1.f);
  gl::glClear(gl::GL_COLOR_BUFFER_BIT);
  
  // draw our first triangle
  gl::glUseProgram(aGLData->shaderProgram);

  // Set uniforms
  auto scale = 10.f / aGLData->dimension.GetRadius();
  aGLData->objectRotation += glm::radians(1.f);
  glm::mat4 model = glm::toMat4(glm::angleAxis(aGLData->objectRotation, glm::vec3{ 0.f, 1.0f, 0.f })) * glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale));
  //glm::mat4 model = glm::mat4(1.0f);

  gl::glUniformMatrix4fv(gl::glGetUniformLocation(aGLData->shaderProgram, "model"), 1, gl::GL_FALSE, &model[0][0]);
  glm::mat4 view = glm::lookAt(aGLData->cameraPosition, glm::vec3(0,0,0), glm::vec3(0,1,0));
  gl::glUniformMatrix4fv(gl::glGetUniformLocation(aGLData->shaderProgram, "view"), 1, gl::GL_FALSE, &view[0][0]);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 10000.0f);

  gl::glUniformMatrix4fv(gl::glGetUniformLocation(aGLData->shaderProgram, "projection"), 1, gl::GL_FALSE, &projection[0][0]);
  gl::glBindVertexArray(aGLData->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
  gl::glDrawElements(gl::GL_TRIANGLE_STRIP, aGLData->Indices, gl::GL_UNSIGNED_INT, 0);

  
  //gl::glUseProgram(aGLData->lineShaderProgram);
  //gl::glDrawElements(gl::GL_LINE_STRIP, aGLData->Indices, gl::GL_UNSIGNED_INT, 0);

  gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
  //gl::glEnable(gl::GL_CULL_FACE);
}































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
  config.aPreferredRenderer = SOIS::PreferredRenderer::OpenGL3_3;
  
  SOIS::ApplicationContext context{config};
  SOIS::ImGuiSample sample;

  std::unique_ptr<LibS06::SonicXNFile> sonic06File;
  FileTracingContext fileContext;

  fileContext.memoryEditor.HighlightFn = HighlightBytes;
  fileContext.memoryEditor.HighlightColor = ImColor(204, 153, 0);

  std::unique_ptr<OpenGLData> drawData;
  
  while (context.Update())
  {
    ImGui::DockSpaceOverViewport();

    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse))
    {
      ImGui::InputText("File", &fileContext.fileName);
      ImGui::InputInt("PositionToBreakOn", &LibS06::gPositionToBreakOn);
      ImGui::InputInt("AddressToBreakOn", &LibS06::gAddressToBreakOn);

      if (drawData.get())
      {
        ImGui::DragFloat3("Camera Position", &drawData->cameraPosition[0]);
      }

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
        drawData = SetUpOpenGLData(sonic06File.get());
      }

      if (sonic06File)
      {
        ShowXnFileInfo(sonic06File.get());

        auto& data = sonic06File->GetReadFile()->GetData();
        
        fileContext.memoryEditor.DrawWindow("Memory Editor", data.data(), data.size());

        ShowTracingInfo(fileContext);

        DrawOpenGLData(drawData);

        ImGui::Image((ImTextureID)drawData->RenderedTexture, ImVec2(800, 800));
      }

      ImGui::End();
    }
  }
  
  return 0;
}

