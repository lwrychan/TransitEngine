#include "render/grid/PhysicalGrid.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "render/grid/GridBackdrop.hpp"
#include "render/grid/GridCamera.hpp"
#include "render/grid/GridInteraction.hpp"
#include "render/grid/GridScene.hpp"
#include "ui/UIStyle.hpp"

namespace render::grid
{
namespace
{
    void applyGridPanelLayout()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        constexpr float margin = 10.0f;
        constexpr float gap = 10.0f;
        constexpr float toolbarOffset = 70.0f;
        const float sideWidth = std::clamp(viewport->WorkSize.x * 0.24f, 260.0f, 320.0f);
        const float gridX = viewport->WorkPos.x + margin + sideWidth + gap;
        const float rightX = viewport->WorkPos.x + viewport->WorkSize.x - margin - sideWidth;
        ImGui::SetNextWindowPos({gridX, viewport->WorkPos.y + margin + toolbarOffset},
                                ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            {std::max(150.0f, rightX - gap - gridX),
             std::max(180.0f, viewport->WorkSize.y - margin * 2.0f - toolbarOffset)},
            ImGuiCond_Always);
    }
} // namespace

void drawPhysicalGridView(core::World& world, const RenderingConfig& config, Tool activeTool,
                          bool modalOpen)
{
    // ImGui redraws this function every frame; these retain editor state between frames.
    static GridCamera camera;
    static GridScene scene;
    static GridInteraction interaction;
    const float minimum = static_cast<float>(std::sqrt(config.minimumGridZoomAreaSquareMeters));
    const float maximum = static_cast<float>(std::sqrt(config.maximumGridZoomAreaSquareMeters));
    camera.setExtent(camera.getExtent(), minimum, maximum);

    applyGridPanelLayout();
    ImGui::Begin("Physical Grid", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    if (ui::smallButton("Reset View"))
    {
        bool hasNodes = false;
        double minimumX = 0.0, maximumX = 0.0, minimumY = 0.0, maximumY = 0.0;
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
            camera.setExtent(minimum, minimum, maximum);
            camera.resetPan();
        }
        else
        {
            const float requiredExtent =
                static_cast<float>(std::max(maximumX - minimumX, maximumY - minimumY) * 1.2 + 2.0);
            camera.setExtent(requiredExtent, minimum, maximum);
            camera.centerOn({(minimumX + maximumX) * 0.5, (minimumY + maximumY) * 0.5, 0.0},
                            ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail());
        }
    }
    float extent = camera.getExtent();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    ui::pushTextInputStyle();
    if (ImGui::InputFloat("Visible Square", &extent, 0.0f, 0.0f, "%.1f m"))
        camera.setExtent(extent, minimum, maximum);
    ui::popTextInputStyle();
    const double visibleAreaSquareMeters =
        static_cast<double>(camera.getExtent()) * camera.getExtent();
    if (visibleAreaSquareMeters >= 1000000.0)
    {
        ImGui::TextDisabled("Viewing %.1f m x %.1f m · (%.2f km²)", camera.getExtent(),
                            camera.getExtent(), visibleAreaSquareMeters / 1000000.0);
    }
    else
    {
        ImGui::TextDisabled("Viewing %.1f m x %.1f m · (%.0f m²)", camera.getExtent(),
                            camera.getExtent(), visibleAreaSquareMeters);
    }
    ImGui::Separator();
    const GridCanvas canvas{ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail(),
                            ImGui::GetWindowDrawList()};
    drawGridBackdrop(camera, canvas);
    const HoverTargets hovered = scene.draw(world, camera, canvas, interaction.getDraggingNode());
    ImGui::InvisibleButton("PhysicalGridCanvas", canvas.size);
    interaction.handle(world, camera, canvas, hovered, activeTool, modalOpen);
    ImGui::End();
}
} // namespace render::grid
