#include "render/grid/PhysicalGrid.hpp"

#include <imgui.h>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "render/grid/GridNavigationControls.hpp"
#include "render/grid/GridPanelLayout.hpp"
#include "render/grid/PhysicalGridViewport.hpp"

namespace render::grid
{
void drawPhysicalGridView(core::World& world, const RenderingConfig& config, Tool activeTool,
                          GridViewMode viewMode, bool modalOpen, bool showDebugPanels)
{
  static PhysicalGridViewport viewport;

  applyPhysicalGridPanelLayout(showDebugPanels);
  ImGui::Begin("Physical Grid", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoCollapse);
  drawGridNavigationControls(world, viewport.getCamera(), config);
  ImGui::Separator();
  viewport.draw(world, activeTool, viewMode, modalOpen);
  ImGui::End();
}
} // namespace render::grid
