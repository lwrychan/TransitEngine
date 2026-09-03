#include "render/grid/GridInteraction.hpp"

#include <algorithm>

#include <imgui.h>

#include "core/World.hpp"
#include "render/GridTool.hpp"
#include "render/grid/GridCamera.hpp"
#include "ui/DebugPanels.hpp"

namespace render::grid
{
void GridInteraction::handle(core::World& world, GridCamera& camera, const GridCanvas& canvas,
                             const HoverTargets& hoverTargets, Tool activeTool, bool modalOpen)
{
    if (modalOpen ||
        !ImGui::IsMouseHoveringRect(
            canvas.position, {canvas.position.x + canvas.size.x, canvas.position.y + canvas.size.y},
            true))
    {
        return;
    }
    if (ImGui::GetIO().MouseWheel != 0.0f)
    {
        camera.zoomAt(ImGui::GetIO().MousePos, canvas.position, canvas.size,
                      ImGui::GetIO().MouseWheel, 5.0f, 5000.0f);
    }
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        camera.panBy(ImGui::GetIO().MouseDelta);
    }
    const network::PhysicalCoordinate mouseWorld =
        camera.screenToWorld(ImGui::GetIO().MousePos, canvas.position, canvas.size);
    const network::PhysicalCoordinate snapped = camera.snapToMeter(mouseWorld);
    if (activeTool == Tool::Node)
    {
        const ImVec2 preview = camera.worldToScreen(snapped, canvas.position, canvas.size);
        canvas.drawList->AddCircleFilled(
            preview, std::clamp(camera.pixelsPerMeter(canvas.size) * 0.18f, 1.5f, 7.0f),
            IM_COL32(0, 0, 0, 105));
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !world.hasNodeAt(snapped))
            ui::requestNodeCreation(snapped);
        return;
    }
    if (activeTool == Tool::Route && hoverTargets.node.has_value() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        ui::toggleRouteNode(*hoverTargets.node);
        return;
    }
    if (hoverTargets.node.has_value() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        activeTool == Tool::Pointer)
    {
        world.setActiveNodeAt(world.getPhysicalNode(*hoverTargets.node).getCoordinate());
        draggingNode = *hoverTargets.node;
        dragStart = ImGui::GetIO().MousePos;
    }
    else if (hoverTargets.node.has_value() && activeTool == Tool::Pointer &&
             (ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
              ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
    {
        world.setActiveNodeAt(world.getPhysicalNode(*hoverTargets.node).getCoordinate());
        ui::requestNodeEdit(world, *hoverTargets.node);
    }
    else if (hoverTargets.route.has_value() && activeTool == Tool::Pointer &&
             (ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
              ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
    {
        world.setActiveRoute(*hoverTargets.route);
        ui::requestRouteEdit(world, *hoverTargets.route);
    }
    else if (hoverTargets.route.has_value() && activeTool == Tool::Pointer &&
             ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        world.setActiveRoute(*hoverTargets.route);
    }
    if (draggingNode.has_value() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        const ImVec2 delta(ImGui::GetIO().MousePos.x - dragStart.x,
                           ImGui::GetIO().MousePos.y - dragStart.y);
        if (delta.x * delta.x + delta.y * delta.y > 16.0f)
        {
            const network::PhysicalCoordinate old =
                world.getPhysicalNode(*draggingNode).getCoordinate();
            ui::requestNodeMove(world, *draggingNode, {snapped.x, snapped.y, old.z});
        }
        draggingNode.reset();
    }
    if (draggingNode.has_value() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        const network::PhysicalCoordinate original =
            world.getPhysicalNode(*draggingNode).getCoordinate();
        const ImVec2 preview =
            camera.worldToScreen({snapped.x, snapped.y, original.z}, canvas.position, canvas.size);
        canvas.drawList->AddCircleFilled(
            preview, std::clamp(camera.pixelsPerMeter(canvas.size) * 0.22f, 4.0f, 9.0f),
            IM_COL32(165, 70, 68, 230));
    }
}
} // namespace render::grid
