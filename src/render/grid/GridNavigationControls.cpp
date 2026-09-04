#include "render/grid/GridNavigationControls.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "render/grid/GridCamera.hpp"
#include "ui/UIStyle.hpp"

namespace render::grid
{
namespace
{
  void resetGridView(core::World& world, GridCamera& camera, float minimumExtent,
                     float maximumExtent)
  {
    bool hasNodes = false;
    double minimumX = 0.0;
    double maximumX = 0.0;
    double minimumY = 0.0;
    double maximumY = 0.0;
    world.forEachNode(
        [&](AbstractNodeId nodeId, network::AbstractNode&)
        {
          const network::PhysicalCoordinate& coordinate =
              world.getPhysicalNode(nodeId).getCoordinate();
          if (!hasNodes)
          {
            minimumX = maximumX = coordinate.x;
            minimumY = maximumY = coordinate.y;
            hasNodes = true;
            return;
          }
          minimumX = std::min(minimumX, coordinate.x);
          maximumX = std::max(maximumX, coordinate.x);
          minimumY = std::min(minimumY, coordinate.y);
          maximumY = std::max(maximumY, coordinate.y);
        });
    if (!hasNodes)
    {
      camera.setExtent(minimumExtent, minimumExtent, maximumExtent);
      camera.resetPan();
      return;
    }

    const float requiredExtent =
        static_cast<float>(std::max(maximumX - minimumX, maximumY - minimumY) * 1.2 + 2.0);
    camera.setExtent(requiredExtent, minimumExtent, maximumExtent);
    camera.centerOn({(minimumX + maximumX) * 0.5, (minimumY + maximumY) * 0.5, 0.0},
                    ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail());
  }
} // namespace

void drawGridNavigationControls(core::World& world, GridCamera& camera,
                                const RenderingConfig& config)
{
  const float minimumExtent = static_cast<float>(std::sqrt(config.minimumGridZoomAreaSquareMeters));
  const float maximumExtent = static_cast<float>(std::sqrt(config.maximumGridZoomAreaSquareMeters));
  camera.setExtent(camera.getExtent(), minimumExtent, maximumExtent);

  if (ui::smallButton("Reset View"))
  {
    resetGridView(world, camera, minimumExtent, maximumExtent);
  }
  float extent = camera.getExtent();
  ImGui::SameLine();
  ImGui::SetNextItemWidth(110.0f);
  ui::pushTextInputStyle();
  if (ImGui::InputFloat("Map Zoom Width", &extent, 0.0f, 0.0f, "%.1f m"))
  {
    camera.setExtent(extent, minimumExtent, maximumExtent);
  }
  ui::popTextInputStyle();

  const double visibleAreaSquareMeters =
      static_cast<double>(camera.getExtent()) * camera.getExtent();
  if (visibleAreaSquareMeters >= 1000000.0)
  {
    ImGui::TextDisabled("Viewing %.1f m x %.1f m · (%.2f km²)", camera.getExtent(),
                        camera.getExtent(), visibleAreaSquareMeters / 1000000.0);
    return;
  }
  ImGui::TextDisabled("Viewing %.1f m x %.1f m · (%.0f m²)", camera.getExtent(), camera.getExtent(),
                      visibleAreaSquareMeters);
}
} // namespace render::grid
