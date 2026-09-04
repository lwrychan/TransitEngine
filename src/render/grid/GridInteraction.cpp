#include "render/grid/GridInteraction.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <imgui.h>

#include "core/World.hpp"
#include "render/GridTool.hpp"
#include "render/grid/GridCamera.hpp"
#include "render/grid/GridGeometryControls.hpp"
#include "render/grid/GridLabels.hpp"
#include "ui/DebugPanels.hpp"

namespace render::grid
{
namespace
{
  double zeroRound(double coordinate)
  {
    const double rounded = std::round(coordinate);
    return rounded == 0.0 ? 0.0 : rounded;
  }
} // namespace

void GridInteraction::handle(core::World& world, GridCamera& camera, const GridCanvas& canvas,
                             const HoverTargets& hoverTargets, Tool activeTool, bool editingEnabled,
                             bool modalOpen)
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
    camera.zoomAt(ImGui::GetIO().MousePos, canvas.position, canvas.size, ImGui::GetIO().MouseWheel,
                  5.0f, 5000.0f);
  }
  const bool altPanning = ImGui::GetIO().KeyAlt && ImGui::IsMouseDown(ImGuiMouseButton_Left);
  if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
      (altPanning && ImGui::IsMouseDragging(ImGuiMouseButton_Left)))
  {
    camera.panBy(ImGui::GetIO().MouseDelta);
  }
  if (altPanning)
  {
    return;
  }
  if (!editingEnabled)
  {
    return;
  }
  const network::PhysicalCoordinate mouseWorld =
      camera.screenToWorld(ImGui::GetIO().MousePos, canvas.position, canvas.size);
  const network::PhysicalCoordinate snapped = {zeroRound(mouseWorld.x), zeroRound(mouseWorld.y),
                                               zeroRound(mouseWorld.z)};
  if (activeTool == Tool::Geometry)
  {
    if (hoverTargets.geometryNode.has_value())
    {
      const HoverTargets::GeometryNode node = *hoverTargets.geometryNode;
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !node.isAnchor)
      {
        world.removeRouteGeometryNode(node.route, node.index);
        clearSelectedGeometrySpan();
        return;
      }
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        world.setActiveRoute(node.route);
        if (!node.isAnchor)
        {
          draggingGeometryNode = node;
          dragStart = ImGui::GetIO().MousePos;
        }
        const network::PhysicalRouteGeometry* geometry = world.getRouteGeometry(node.route);
        if (geometry != nullptr && !geometry->spans.empty())
        {
          selectGeometrySpan(
              {.route = node.route, .index = std::min(node.index, geometry->spans.size() - 1)});
        }
        return;
      }
    }
    if (hoverTargets.geometrySpan.has_value() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      const HoverTargets::GeometrySpan span = *hoverTargets.geometrySpan;
      selectGeometrySpan(span);
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
      {
        world.insertRouteGeometryNode(span.route, span.index, snapped);
      }
      return;
    }
    if (hoverTargets.route.has_value() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      world.setActiveRoute(*hoverTargets.route);
      clearSelectedGeometrySpan();
      return;
    }
    if (draggingGeometryNode.has_value() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
      const HoverTargets::GeometryNode node = *draggingGeometryNode;
      const network::PhysicalRouteGeometry* geometry = world.getRouteGeometry(node.route);
      if (geometry != nullptr && node.index < geometry->nodes.size())
      {
        const double z = geometry->nodes[node.index].coordinate.z;
        world.moveRouteGeometryNode(node.route, node.index, {snapped.x, snapped.y, z});
      }
      draggingGeometryNode.reset();
    }
    if (draggingGeometryNode.has_value() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
      const HoverTargets::GeometryNode node = *draggingGeometryNode;
      const network::PhysicalRouteGeometry* geometry = world.getRouteGeometry(node.route);
      if (geometry != nullptr && node.index < geometry->nodes.size())
      {
        const network::PhysicalCoordinate& original = geometry->nodes[node.index].coordinate;
        const ImVec2 preview =
            camera.worldToScreen({snapped.x, snapped.y, original.z}, canvas.position, canvas.size);
        canvas.drawList->AddCircleFilled(preview, 6.0f, IM_COL32(88, 209, 255, 110), 12);
        canvas.drawList->AddCircle(preview, 7.0f, IM_COL32(88, 209, 255, 230), 12, 1.5f);
      }
    }
    return;
  }
  if (activeTool == Tool::Node)
  {
    const ImVec2 preview = camera.worldToScreen(snapped, canvas.position, canvas.size);
    canvas.drawList->AddCircleFilled(
        preview, std::clamp(camera.pixelsPerMeter(canvas.size) * 0.18f, 1.5f, 7.0f),
        IM_COL32(0, 0, 0, 105));
    char coordinateText[64];
    std::snprintf(coordinateText, sizeof(coordinateText), "(%.0f, %.0f, %.0f) m", snapped.x,
                  snapped.y, snapped.z);
    const ImVec2 textSize = ImGui::CalcTextSize(coordinateText);
    const ImVec2 canvasMaximum(canvas.position.x + canvas.size.x,
                               canvas.position.y + canvas.size.y);
    const ImVec2 textPosition(
        std::clamp(preview.x + 10.0f, canvas.position.x, canvasMaximum.x - textSize.x - 6.0f),
        std::clamp(preview.y + 10.0f, canvas.position.y, canvasMaximum.y - textSize.y - 4.0f));
    drawReadableLabel(*canvas.drawList, textPosition, coordinateText);
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
      const network::PhysicalCoordinate old = world.getPhysicalNode(*draggingNode).getCoordinate();
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
