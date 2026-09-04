#include "render/grid/PhysicalGridViewport.hpp"

#include <imgui.h>

#include "core/World.hpp"
#include "render/grid/GridBackdrop.hpp"
#include "render/grid/GridGeometryControls.hpp"

namespace render::grid
{
GridCamera& PhysicalGridViewport::getCamera()
{
  return camera;
}

const GridInteraction& PhysicalGridViewport::getInteraction() const
{
  return interaction;
}

void PhysicalGridViewport::draw(core::World& world, Tool activeTool, GridViewMode viewMode,
                                bool modalOpen)
{
  const GridCanvas canvas{ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail(),
                          ImGui::GetWindowDrawList()};
  drawGridBackdrop(camera, canvas, viewMode);
  const HoverTargets hovered = scene.draw(world, camera, canvas, interaction.getDraggingNode(),
                                          interaction.getDraggingGeometryNode(),
                                          getSelectedGeometrySpan(), viewMode, activeTool);
  ImGui::InvisibleButton("PhysicalGridCanvas", canvas.size);
  interaction.handle(world, camera, canvas, hovered, activeTool, viewMode == GridViewMode::Plan,
                     modalOpen);
  if (viewMode == GridViewMode::Elevation)
  {
    ImGui::SetCursorScreenPos({canvas.position.x + 12.0f, canvas.position.y + 12.0f});
    ImGui::TextDisabled("Axonometric 2.5D view · Z layers · editing disabled");
  }
}
} // namespace render::grid
