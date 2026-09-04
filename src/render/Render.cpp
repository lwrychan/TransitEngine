#include "render/Render.hpp"
#include "Resources.hpp"
#include "Simulation.hpp"
#include "core/World.hpp"
#include "render/EditorLayout.hpp"
#include "render/grid/GridGeometryControls.hpp"
#include "render/grid/PhysicalGrid.hpp"
#include "ui/DebugPanels.hpp"
#include "ui/UIStyle.hpp"

#include <algorithm>
#include <chrono>

namespace
{
enum class ToolIcon
{
  MousePointer,
  CircleDot,
  Route,
  BezierCurve
};

constexpr float ToolButtonSize = 32.0f;

// Lucide: MousePointer2, CircleDot, and Route. Rendered as paths to avoid a font dependency.
void drawToolIcon(ImDrawList& drawList, ImVec2 minimum, ImVec2 maximum, ToolIcon icon)
{
  const ImU32 color = IM_COL32(255, 255, 255, 255);
  const ImVec2 center((minimum.x + maximum.x) * 0.5f, (minimum.y + maximum.y) * 0.5f);

  switch (icon)
  {
  case ToolIcon::MousePointer:
  {
    const ImVec2 pointer[] = {{center.x - 6.0f, center.y - 6.0f},
                              {center.x - 0.7f, center.y + 6.75f},
                              {center.x + 1.18f, center.y + 1.21f},
                              {center.x + 6.75f, center.y - 0.7f}};
    drawList.AddPolyline(pointer, IM_ARRAYSIZE(pointer), color, ImDrawFlags_Closed, 1.7f);
    drawList.AddLine({center.x + 0.75f, center.y + 0.75f}, {center.x + 5.25f, center.y + 5.25f},
                     color, 1.7f);
    break;
  }
  case ToolIcon::CircleDot:
    drawList.AddCircle(center, 7.5f, color, 16, 1.7f);
    drawList.AddCircleFilled(center, 1.5f, color, 12);
    break;
  case ToolIcon::Route:
  {
    const ImVec2 start(center.x - 4.5f, center.y + 5.25f);
    const ImVec2 end(center.x + 4.5f, center.y - 5.25f);
    drawList.AddLine({start.x + 2.25f, start.y}, {center.x + 2.625f, start.y}, color, 1.7f);
    drawList.AddBezierCubic({center.x + 2.625f, start.y}, {center.x + 7.875f, start.y},
                            {center.x + 7.875f, center.y}, {center.x + 2.625f, center.y}, color,
                            1.7f);
    drawList.AddLine({center.x + 2.625f, center.y}, {center.x - 4.5f, center.y}, color, 1.7f);
    drawList.AddBezierCubic({center.x - 4.5f, center.y}, {center.x - 9.75f, center.y},
                            {center.x - 9.75f, center.y - 5.25f},
                            {center.x - 4.5f, center.y - 5.25f}, color, 1.7f);
    drawList.AddLine({center.x - 4.5f, center.y - 5.25f}, {end.x - 2.25f, end.y}, color, 1.7f);
    drawList.AddCircle(start, 2.25f, color, 12, 1.7f);
    drawList.AddCircle(end, 2.25f, color, 12, 1.7f);
    break;
  }
  case ToolIcon::BezierCurve:
    drawList.AddBezierCubic({center.x - 7.0f, center.y + 5.0f}, {center.x - 2.0f, center.y - 8.0f},
                            {center.x + 3.0f, center.y + 8.0f}, {center.x + 7.0f, center.y - 5.0f},
                            color, 1.75f);
    drawList.AddCircleFilled({center.x - 7.0f, center.y + 5.0f}, 1.8f, color);
    drawList.AddCircleFilled({center.x + 7.0f, center.y - 5.0f}, 1.8f, color);
    break;
  }
}

bool selectedButton(const char* label, ImVec2 size, bool selected, const char* tooltip)
{
  if (selected)
  {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.36f, 0.07f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.44f, 0.10f, 0.13f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.47f, 0.49f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
  }

  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
  const bool clicked = ImGui::Button(label, size);
  ImGui::PopStyleColor();
  if (selected)
  {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
  }

  if (ImGui::IsItemHovered())
  {
    ImGui::SetTooltip("%s", tooltip);
  }
  return clicked;
}

bool toolButton(const char* id, ToolIcon icon, bool selected, const char* tooltip)
{
  const bool clicked = selectedButton(id, {ToolButtonSize, ToolButtonSize}, selected, tooltip);
  drawToolIcon(*ImGui::GetWindowDrawList(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), icon);
  return clicked;
}

void applyMetroTheme(ImGuiStyle& style)
{
  style.Colors[ImGuiCol_Button] = ImVec4(0.78f, 0.20f, 0.23f, 1.0f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.88f, 0.31f, 0.34f, 1.0f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.66f, 0.10f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.78f, 0.20f, 0.23f, 1.0f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.88f, 0.31f, 0.34f, 1.0f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.66f, 0.10f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_Tab] = ImVec4(0.70f, 0.15f, 0.18f, 1.0f);
  style.Colors[ImGuiCol_TabHovered] = ImVec4(0.88f, 0.31f, 0.34f, 1.0f);
  style.Colors[ImGuiCol_TabActive] = ImVec4(0.66f, 0.10f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.62f, 0.16f, 0.19f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.74f, 0.21f, 0.24f, 1.0f);
}
} // namespace

void render::Render::drawToolBar(core::World& world)
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const float menuHeight = ImGui::GetFrameHeight();
  constexpr float verticalPadding = (editorlayout::toolBarHeight - ToolButtonSize) * 0.5f;

  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + editorlayout::outerMargin,
                                 viewport->Pos.y + menuHeight + editorlayout::toolBarTopGap),
                          ImGuiCond_Always);
  ImGui::SetNextWindowSize(
      ImVec2(std::max(180.0f, viewport->WorkSize.x - editorlayout::outerMargin * 2.0f),
             editorlayout::toolBarHeight),
      ImGuiCond_Always);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, verticalPadding));
  ImGui::Begin("Tools", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse);

  const auto selectTool = [&](Tool tool)
  {
    if (activeTool == tool)
    {
      return;
    }
    activeTool = tool;
    world.clearActiveSelection();
    ui::clearRouteNodeSelection();
    grid::clearSelectedGeometrySpan();
  };

  if (toolButton("##pointer-tool", ToolIcon::MousePointer, activeTool == Tool::Pointer,
                 "Pointer tool"))
  {
    selectTool(Tool::Pointer);
  }

  ImGui::SameLine();
  if (toolButton("##node-tool", ToolIcon::CircleDot, activeTool == Tool::Node, "Node tool"))
  {
    selectTool(Tool::Node);
  }

  ImGui::SameLine();
  if (toolButton("##route-tool", ToolIcon::Route, activeTool == Tool::Route, "Route tool"))
  {
    selectTool(Tool::Route);
  }

  ImGui::SameLine();
  if (toolButton("##geometry-tool", ToolIcon::BezierCurve, activeTool == Tool::Geometry,
                 "Geometry editor"))
  {
    selectTool(Tool::Geometry);
  }

  ImGui::SameLine(0.0f, 12.0f);
  if (selectedButton("2.5D##elevation-view", {48.0f, ToolButtonSize},
                     gridViewMode == grid::GridViewMode::Elevation, "Toggle 2.5D elevation view"))
  {
    gridViewMode = gridViewMode == grid::GridViewMode::Plan ? grid::GridViewMode::Elevation
                                                            : grid::GridViewMode::Plan;
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void render::Render::drawAboutModal()
{
  if (!ImGui::BeginPopupModal("About TransitEngine", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
  {
    return;
  }

  ImGui::TextWrapped(
      "TransitEngine is a C++23 desktop prototype for building small transit networks and "
      "simulating vehicles.");
  ImGui::Spacing();
  ImGui::TextWrapped(
      "SDL2, OpenGL, and Dear ImGui provide an interactive physical-grid editor; the simulation "
      "advances the editable world at a fixed rate.");
  ImGui::Spacing();
  ImGui::TextWrapped(
      "It is an editor and simulation foundation, not yet a full transit-planning application.");

  ImGui::Separator();
  if (ui::button("Open GitHub Repository"))
  {
    openURL("https://github.com/lwrychan/TransitEngine");
  }
  ImGui::SameLine();
  if (ui::button("Close"))
  {
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
}

float render::Render::pxFromPt(float pt, float dpi)
{
  // With 96 DPI: px = pt * DPI / 72. ~ px = pt * 4/3
  return pt * dpi / 72;
}

void render::Render::openURL(const std::string& url)
{
#ifdef _WIN32
  std::string command = "start " + url;
#elif __APPLE__
  std::string command = "open " + url;
#elif __linux__
  std::string command = "xdg-open " + url;
#endif
  std::system(command.c_str());
}

void render::Render::setup()
{
  Resources resources = Resources();
  // Initial SDL2, ImGui, and OpenGL setup
  SDL_Init(SDL_INIT_VIDEO);

  char* basePath = SDL_GetBasePath();
  this->resourcesPath = std::filesystem::path(basePath ? basePath : "");

  SDL_free(basePath);

  this->executableDirectory = this->resourcesPath / ".." / "MacOS";
  this->fontResourcesPath = this->resourcesPath / "fonts" / "GoogleSansFlex.ttf";

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  this->window =
      SDL_CreateWindow("Transit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  this->glContext = SDL_GL_CreateContext(this->window);

  SDL_GL_MakeCurrent(this->window, this->glContext);
  SDL_GL_SetSwapInterval(1); // VSync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Window and ImGui context created.
  std::cout << "Window and ImGui context created.";
  std::cout << "Base (executable) directory: " << this->executableDirectory << std::endl;

  ImGui::StyleColorsLight();
  applyMetroTheme(ImGui::GetStyle());
  // ImGui::StyleColorsDark();

  std::cout << "Set background to light theme.";

  // IO Settings
  std::cout << "Loading fonts...\n" << std::endl;

  ImGuiIO& ioSettings = ImGui::GetIO();
  ioSettings.FontGlobalScale = 1.0f;
  ioSettings.Fonts->AddFontFromFileTTF(this->fontResourcesPath.string().c_str(),
                                       render::Render::pxFromPt(16.0));

  std::cout << "Resource directory: " << this->resourcesPath << std::endl;
  // =============

  ImGui_ImplSDL2_InitForOpenGL(window, glContext);
  ImGui_ImplOpenGL3_Init("#version 330");

  ui::LogBuffer::instance().add("Transit Engine initialized.");
}

double render::Render::getLastRenderWorkMs() const
{
  return this->lastRenderWorkMs;
}

bool render::Render::update(Simulation& simulation)
{
  const auto renderWorkStart = std::chrono::high_resolution_clock::now();

  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT)
      return false;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  bool showAboutPopup = false;
  bool showPopupManageVehicles = false;

  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("About"))
    {
      if (ImGui::MenuItem("About TransitEngine"))
      {
        showAboutPopup = true;
      }
      if (ImGui::MenuItem("GitHub Repository"))
      {
        render::Render::openURL("https://github.com/lwrychan/TransitEngine");
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Simulation"))
    {
      if (ImGui::MenuItem("Start / Resume"))
      {
        simulation.setRunning(true);
        ui::LogBuffer::instance().add("Simulation started from menu.");
      }

      if (ImGui::MenuItem("Pause"))
      {
        simulation.setRunning(false);
        ui::LogBuffer::instance().add("Simulation paused from menu.");
      }

      if (ImGui::BeginMenu("Simulation Rate"))
      {
        if (ImGui::MenuItem("0.25x"))
        {
          simulation.setSpeedMultiplier(0.25);
        }

        if (ImGui::MenuItem("1x"))
        {
          simulation.setSpeedMultiplier(1.0);
        }

        if (ImGui::MenuItem("2x"))
        {
          simulation.setSpeedMultiplier(2.0);
        }

        if (ImGui::MenuItem("4x"))
        {
          simulation.setSpeedMultiplier(4.0);
        }

        ImGui::EndMenu();
      }

      if (ImGui::MenuItem("Manage Vehicles"))
      {
        showPopupManageVehicles = true;
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Add"))
    {
      if (ImGui::BeginMenu("Node"))
      {
        if (ImGui::BeginMenu("NodeMenu"))
        {
          if (ImGui::MenuItem("Rail Station"))
          {
          }
          if (ImGui::MenuItem("Bus Stop"))
          {
          }
          if (ImGui::MenuItem("Road Intersection"))
          {
          }

          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
      ImGui::MenuItem("Undo");
      ImGui::MenuItem("Redo");

      ImGui::MenuItem("Copy");
      ImGui::MenuItem("Paste");

      ImGui::MenuItem("Rotate Clockwise");
      ImGui::MenuItem("Rotate Counterclockwise");
      ImGui::MenuItem("Set Rotation");

      ImGui::MenuItem("Properties");

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
      ImGui::MenuItem("Show Routes");
      ImGui::MenuItem("Show Vehicles");
      ImGui::Separator();
      ImGui::MenuItem("Debug View", nullptr, &debugView);

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help"))
    {
      if (ImGui::MenuItem("Documentation"))
      {
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  if (showAboutPopup)
  {
    ImGui::OpenPopup("About TransitEngine");
  }
  drawAboutModal();
  ui::drawEditorDialogs(simulation);
  ui::drawSimulationControls(simulation, debugView);
  ui::drawInspector(simulation, debugView);
  ui::drawEditorPanel(simulation, debugView);
  if (debugView)
  {
    ui::drawPerformancePanel(simulation);
    ui::drawWorldSummary(simulation);
    ui::drawLogPanel();
  }
  const bool modalOpen = ui::isModalOpen() || ui::isRouteModalOpen() || showPopupManageVehicles ||
                         ImGui::IsPopupOpen("Manage Vehicles");
  render::grid::drawPhysicalGridView(simulation.getWorld(), simulation.getConfig().renderingConfig,
                                     activeTool, gridViewMode, modalOpen, debugView);
  ui::drawRouteControls(simulation.getWorld(), activeTool);

  if (ImGui::BeginPopupModal("Manage Vehicles", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::Text("Vehicle management");
    ImGui::Text("Create a vehicle for the current simulation.");
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::Button("Create Vehicle", ImVec2(120, 0)))
    {
      ui::requestVehicleCreation();
      ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor();

    ImGui::EndPopup();
  }

  if (showPopupManageVehicles)
  {
    ImGui::OpenPopup("Manage Vehicles");
  }

  drawToolBar(simulation.getWorld());

  ImGui::Render();

  // OpenGL rendering

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  const auto renderWorkEnd = std::chrono::high_resolution_clock::now();
  this->lastRenderWorkMs =
      std::chrono::duration<double>(renderWorkEnd - renderWorkStart).count() * 1000.0;

  SDL_GL_SwapWindow(window);

  return true;
}

void render::Render::close()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();

  ImGui::DestroyContext();

  SDL_GL_DeleteContext(this->glContext);
  SDL_DestroyWindow(this->window);

  SDL_Quit();
}
