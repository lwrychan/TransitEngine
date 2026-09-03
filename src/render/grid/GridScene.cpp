#include "render/grid/GridScene.hpp"

#include <algorithm>
#include <cmath>

#include "core/World.hpp"
#include "network/NodeType.hpp"
#include "render/grid/GridCamera.hpp"
#include "render/grid/GridLabels.hpp"
#include "ui/DebugPanels.hpp"
#include "vehicle/Vehicle.hpp"

namespace render::grid
{
HoverTargets GridScene::draw(core::World& world, const GridCamera& camera, const GridCanvas& canvas,
                             const std::optional<AbstractNodeId>& draggingNode)
{
    HoverTargets hovered;
    if (canvas.drawList == nullptr)
    {
        return hovered;
    }

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const float pixelsPerMeter = camera.pixelsPerMeter(canvas.size);
    const ImVec2 canvasMaximum(canvas.position.x + canvas.size.x,
                               canvas.position.y + canvas.size.y);
    std::vector<std::pair<ImVec2, ImVec2>> routeSegments;
    std::vector<ImRect> placedLabels;
    canvas.drawList->PushClipRect(canvas.position, canvasMaximum, true);
    const auto updateRouteHover =
        [&](AbstractRouteId routeId, ImVec2 start, ImVec2 end, float width)
    {
        const float deltaX = end.x - start.x;
        const float deltaY = end.y - start.y;
        const float lengthSquared = deltaX * deltaX + deltaY * deltaY;
        if (lengthSquared <= 0.0f)
            return;
        const float factor = std::clamp(
            ((mouse.x - start.x) * deltaX + (mouse.y - start.y) * deltaY) / lengthSquared, 0.0f,
            1.0f);
        const float closestX = start.x + factor * deltaX;
        const float closestY = start.y + factor * deltaY;
        const float offsetX = mouse.x - closestX;
        const float offsetY = mouse.y - closestY;
        if (offsetX * offsetX + offsetY * offsetY <= (width + 6.0f) * (width + 6.0f))
            hovered.route = routeId;
    };
    world.forEachRoute(
        [&](AbstractRouteId routeId, network::AbstractRoute& route)
        {
            const auto& nodes = route.getNodes();
            const network::RouteColor& color = route.getColor();
            const ImU32 routeColor =
                IM_COL32(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
                         static_cast<int>(color.b * 255), 235);
            const bool selected = world.getActiveRoute().has_value() &&
                                  world.getActiveRoute()->id == routeId.id &&
                                  world.getActiveRoute()->generation == routeId.generation;
            const float width =
                std::clamp(pixelsPerMeter * 0.20f, 2.5f, 6.0f) + (selected ? 2.0f : 0.0f);
            ImVec2 longestStart{};
            ImVec2 longestEnd{};
            ImVec2 centroid{};
            float longestLengthSquared = 0.0f;
            for (size_t index = 1; index < nodes.size(); ++index)
            {
                const ImVec2 start =
                    camera.worldToScreen(world.getPhysicalNode(nodes[index - 1]).getCoordinate(),
                                         canvas.position, canvas.size);
                const ImVec2 end =
                    camera.worldToScreen(world.getPhysicalNode(nodes[index]).getCoordinate(),
                                         canvas.position, canvas.size);
                canvas.drawList->AddLine(start, end, routeColor, width);
                routeSegments.emplace_back(start, end);
                updateRouteHover(routeId, start, end, width);
                const float deltaX = end.x - start.x;
                const float deltaY = end.y - start.y;
                const float lengthSquared = deltaX * deltaX + deltaY * deltaY;
                if (lengthSquared > longestLengthSquared)
                {
                    longestLengthSquared = lengthSquared;
                    longestStart = start;
                    longestEnd = end;
                }
            }
            if (!route.getName().empty() && longestLengthSquared >= 900.0f)
            {
                for (AbstractNodeId nodeId : nodes)
                {
                    const ImVec2 point =
                        camera.worldToScreen(world.getPhysicalNode(nodeId).getCoordinate(),
                                             canvas.position, canvas.size);
                    centroid.x += point.x;
                    centroid.y += point.y;
                }
                centroid.x /= static_cast<float>(nodes.size());
                centroid.y /= static_cast<float>(nodes.size());
                const float length = std::sqrt(longestLengthSquared);
                ImVec2 direction{(longestEnd.x - longestStart.x) / length,
                                 (longestEnd.y - longestStart.y) / length};
                ImVec2 normal{-direction.y, direction.x};
                const ImVec2 midpoint{(longestStart.x + longestEnd.x) * 0.5f,
                                      (longestStart.y + longestEnd.y) * 0.5f};
                if ((centroid.x - midpoint.x) * normal.x + (centroid.y - midpoint.y) * normal.y <
                    0.0f)
                    normal = {-normal.x, -normal.y};
                if (direction.x < 0.0f)
                    direction = {-direction.x, -direction.y};
                drawRotatedRouteLabel(*canvas.drawList,
                                      {midpoint.x + normal.x * (width + 8.0f),
                                       midpoint.y + normal.y * (width + 8.0f)},
                                      direction, IM_COL32(20, 30, 40, 255),
                                      route.getName().c_str());
            }
        });
    world.forEachVehicle(
        [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
        {
            const auto pose = world.getVehiclePose(vehicleId);
            if (!pose.has_value())
                return;
            const ImVec2 position =
                camera.worldToScreen(pose->coordinate, canvas.position, canvas.size);
            const ImVec2 direction(static_cast<float>(pose->direction.x),
                                   static_cast<float>(-pose->direction.y));
            const ImVec2 normal(-direction.y, direction.x);
            const float halfLength = std::clamp(pixelsPerMeter * 1.1f, 9.0f, 34.0f);
            const float halfWidth = std::clamp(pixelsPerMeter * 0.45f, 5.0f, 14.0f);
            const ImVec2 frontLeft(position.x + direction.x * halfLength + normal.x * halfWidth,
                                   position.y + direction.y * halfLength + normal.y * halfWidth);
            const ImVec2 frontRight(position.x + direction.x * halfLength - normal.x * halfWidth,
                                    position.y + direction.y * halfLength - normal.y * halfWidth);
            const ImVec2 rearRight(position.x - direction.x * halfLength - normal.x * halfWidth,
                                   position.y - direction.y * halfLength - normal.y * halfWidth);
            const ImVec2 rearLeft(position.x - direction.x * halfLength + normal.x * halfWidth,
                                  position.y - direction.y * halfLength + normal.y * halfWidth);
            canvas.drawList->AddQuadFilled(frontLeft, frontRight, rearRight, rearLeft,
                                           IM_COL32(32, 61, 92, 255));
            canvas.drawList->AddQuad(frontLeft, frontRight, rearRight, rearLeft,
                                     IM_COL32(245, 248, 250, 255), 1.5f);
            canvas.drawList->AddLine(frontLeft, frontRight, IM_COL32(90, 196, 232, 255), 2.0f);
            const float radius = std::max(halfLength, halfWidth);
            const float deltaX = mouse.x - position.x;
            const float deltaY = mouse.y - position.y;
            if (deltaX * deltaX + deltaY * deltaY <= radius * radius)
                hovered.vehicle = vehicleId;
            const std::string label = ui::shouldShowVehicleSpeeds()
                                          ? vehicle.getDisplayName() + " (" +
                                                std::to_string(static_cast<int>(
                                                    std::round(vehicle.getCurrentSpeedKph()))) +
                                                " km/h)"
                                          : vehicle.getDisplayName();
            const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            const ImVec2 labelPosition =
                chooseClearLabelPosition(position, textSize, radius + 8.0f, canvas.position,
                                         canvasMaximum, routeSegments, placedLabels);
            drawReadableLabel(*canvas.drawList, labelPosition, label.c_str());
            placedLabels.emplace_back(
                labelPosition, ImVec2(labelPosition.x + textSize.x, labelPosition.y + textSize.y));
        });
    world.forEachNode(
        [&](AbstractNodeId nodeId, network::AbstractNode& node)
        {
            const ImVec2 position = camera.worldToScreen(
                world.getPhysicalNode(nodeId).getCoordinate(), canvas.position, canvas.size);
            const bool selected = world.getActiveNode().has_value() &&
                                  world.getActiveNode()->id == nodeId.id &&
                                  world.getActiveNode()->generation == nodeId.generation;
            const bool selectedForRoute = ui::isRouteNodeSelected(nodeId);
            const bool dragging = draggingNode.has_value() && draggingNode->id == nodeId.id &&
                                  draggingNode->generation == nodeId.generation;
            const float radius = std::clamp(pixelsPerMeter * 0.22f, 4.0f, 9.0f);
            canvas.drawList->AddCircleFilled(
                position, radius,
                selectedForRoute ? IM_COL32(170, 48, 56, dragging ? 80 : 255)
                : selected       ? IM_COL32(190, 55, 63, dragging ? 80 : 255)
                                 : IM_COL32(112, 116, 120, dragging ? 80 : 230),
                20);
            if (selected)
                canvas.drawList->AddCircle(position, radius + 4.0f, IM_COL32(255, 205, 70, 255), 20,
                                           2.0f);
            if (selectedForRoute)
                canvas.drawList->AddCircle(position, radius + 4.0f, IM_COL32(115, 205, 255, 255),
                                           20, 2.0f);
            const float deltaX = mouse.x - position.x;
            const float deltaY = mouse.y - position.y;
            if (deltaX * deltaX + deltaY * deltaY <= 100.0f)
            {
                hovered.node = nodeId;
            }
            const network::PhysicalNode& physicalNode = world.getPhysicalNode(nodeId);
            const std::string label =
                node.getName() + " (" + network::nodeTypeName(physicalNode.getNodeType()) + ")";
            const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            const ImVec2 labelPosition =
                chooseClearLabelPosition(position, textSize, 16.0f, canvas.position, canvasMaximum,
                                         routeSegments, placedLabels);
            drawReadableLabel(*canvas.drawList, labelPosition, label.c_str());
            placedLabels.emplace_back(
                labelPosition, ImVec2(labelPosition.x + textSize.x, labelPosition.y + textSize.y));
        });
    canvas.drawList->PopClipRect();
    if (hovered.node.has_value())
    {
        const network::AbstractNode& node = world.getNode(*hovered.node);
        const network::PhysicalNode& physicalNode = world.getPhysicalNode(*hovered.node);
        const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();
        ImGui::SetTooltip("%s\nType: %s\nCoordinate: (%.0f, %.0f, %.0f) m", node.getName().c_str(),
                          network::nodeTypeName(physicalNode.getNodeType()), coordinate.x,
                          coordinate.y, coordinate.z);
    }
    else if (hovered.vehicle.has_value())
    {
        world.forEachVehicle(
            [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
            {
                if (vehicleId.id == hovered.vehicle->id &&
                    vehicleId.generation == hovered.vehicle->generation)
                    ImGui::SetTooltip("%s\nCurrent speed: %.1f km/h",
                                      vehicle.getDisplayName().c_str(),
                                      vehicle.getCurrentSpeedKph());
            });
    }
    return hovered;
}
} // namespace render::grid
