#include "render/grid/GridScene.hpp"

#include <algorithm>
#include <cmath>

#include "core/World.hpp"
#include "network/NodeType.hpp"
#include "network/RouteGeometry.hpp"
#include "render/grid/GridCamera.hpp"
#include "render/grid/GridLabels.hpp"
#include "render/grid/GridProjection.hpp"
#include "ui/DebugPanels.hpp"
#include "vehicle/Vehicle.hpp"

namespace render::grid
{
namespace
{
  float nodeMarkerRadius(float pixelsPerMeter)
  {
    return std::clamp(pixelsPerMeter * 0.22f, 4.0f, 9.0f);
  }

  bool sameRoute(AbstractRouteId left, AbstractRouteId right)
  {
    return left.id == right.id && left.generation == right.generation;
  }

  void drawVehiclePrism(ImDrawList& drawList, const GridProjection& projection,
                        const core::PhysicalVehiclePose& pose)
  {
    constexpr double halfLengthMeters = 1.1;
    constexpr double halfWidthMeters = 0.45;
    constexpr double bodyHeightMeters = 0.9;

    // Keep the body upright on grades. The vehicle position follows Z, while its footprint uses
    // the horizontal track heading so a steep route cannot visually compress the prism.
    const double horizontalDirectionLength = std::hypot(pose.direction.x, pose.direction.y);
    const network::PhysicalCoordinate forward =
        horizontalDirectionLength > 0.0
            ? network::PhysicalCoordinate{pose.direction.x / horizontalDirectionLength,
                                          pose.direction.y / horizontalDirectionLength, 0.0}
            : network::PhysicalCoordinate{1.0, 0.0, 0.0};
    const network::PhysicalCoordinate side =
        horizontalDirectionLength > 0.0 ? network::PhysicalCoordinate{-forward.y, forward.x, 0.0}
                                        : network::PhysicalCoordinate{0.0, 1.0, 0.0};
    const auto corner = [&](double longitudinal, double lateral, double vertical)
    {
      return network::PhysicalCoordinate{
          pose.coordinate.x + forward.x * longitudinal + side.x * lateral,
          pose.coordinate.y + forward.y * longitudinal + side.y * lateral,
          pose.coordinate.z + vertical};
    };

    const ImVec2 frontLeft =
        projection.worldToScreen(corner(halfLengthMeters, halfWidthMeters, 0.0));
    const ImVec2 frontRight =
        projection.worldToScreen(corner(halfLengthMeters, -halfWidthMeters, 0.0));
    const ImVec2 rearRight =
        projection.worldToScreen(corner(-halfLengthMeters, -halfWidthMeters, 0.0));
    const ImVec2 rearLeft =
        projection.worldToScreen(corner(-halfLengthMeters, halfWidthMeters, 0.0));
    const ImVec2 topFrontLeft =
        projection.worldToScreen(corner(halfLengthMeters, halfWidthMeters, bodyHeightMeters));
    const ImVec2 topFrontRight =
        projection.worldToScreen(corner(halfLengthMeters, -halfWidthMeters, bodyHeightMeters));
    const ImVec2 topRearRight =
        projection.worldToScreen(corner(-halfLengthMeters, -halfWidthMeters, bodyHeightMeters));
    const ImVec2 topRearLeft =
        projection.worldToScreen(corner(-halfLengthMeters, halfWidthMeters, bodyHeightMeters));

    // The axonometric camera sees the roof and the two faces whose normals point toward it.
    constexpr network::PhysicalCoordinate cameraDirection{1.0, 1.0, 1.1764706};
    const double forwardVisibility = forward.x * cameraDirection.x + forward.y * cameraDirection.y;
    const double sideVisibility = side.x * cameraDirection.x + side.y * cameraDirection.y;
    if (forwardVisibility >= 0.0)
    {
      drawList.AddQuadFilled(frontLeft, frontRight, topFrontRight, topFrontLeft,
                             IM_COL32(21, 43, 66, 255));
    }
    else
    {
      drawList.AddQuadFilled(rearRight, rearLeft, topRearLeft, topRearRight,
                             IM_COL32(21, 43, 66, 255));
    }
    if (sideVisibility >= 0.0)
    {
      drawList.AddQuadFilled(frontLeft, rearLeft, topRearLeft, topFrontLeft,
                             IM_COL32(35, 69, 102, 255));
    }
    else
    {
      drawList.AddQuadFilled(rearRight, frontRight, topFrontRight, topRearRight,
                             IM_COL32(35, 69, 102, 255));
    }
    drawList.AddQuadFilled(topFrontLeft, topFrontRight, topRearRight, topRearLeft,
                           IM_COL32(53, 102, 145, 255));
    drawList.AddQuad(topFrontLeft, topFrontRight, topRearRight, topRearLeft,
                     IM_COL32(245, 248, 250, 255), 1.25f);
    drawList.AddLine(frontLeft, topFrontLeft, IM_COL32(245, 248, 250, 255), 1.25f);
    drawList.AddLine(frontRight, topFrontRight, IM_COL32(245, 248, 250, 255), 1.25f);
    drawList.AddLine(rearRight, topRearRight, IM_COL32(245, 248, 250, 255), 1.25f);
    drawList.AddLine(rearLeft, topRearLeft, IM_COL32(245, 248, 250, 255), 1.25f);
    drawList.AddLine(topFrontLeft, topFrontRight, IM_COL32(90, 196, 232, 255), 2.0f);
  }

  using RouteSegment = std::pair<ImVec2, ImVec2>;

  struct SceneDrawContext
  {
    core::World& world;
    const GridCanvas& canvas;
    const GridProjection& projection;
    ImDrawList& drawList;
    ImVec2 mouse;
    ImVec2 canvasMaximum;
    float pixelsPerMeter;
    const std::optional<AbstractNodeId>& draggingNode;
    const std::optional<HoverTargets::GeometryNode>& draggingGeometryNode;
    const std::optional<HoverTargets::GeometrySpan>& selectedGeometrySpan;
    Tool activeTool;
    HoverTargets hovered;
    std::vector<RouteSegment> routeSegments;
    std::vector<ImRect> placedLabels;
  };

} // namespace

HoverTargets GridScene::draw(core::World& world, const GridCamera& camera, const GridCanvas& canvas,
                             const std::optional<AbstractNodeId>& draggingNode,
                             const std::optional<HoverTargets::GeometryNode>& draggingGeometryNode,
                             const std::optional<HoverTargets::GeometrySpan>& selectedGeometrySpan,
                             GridViewMode viewMode, Tool activeTool)
{
  HoverTargets hovered;
  if (canvas.drawList == nullptr)
  {
    return hovered;
  }

  const GridProjection projection(camera, canvas.position, canvas.size, viewMode);
  SceneDrawContext context{world,
                           canvas,
                           projection,
                           *canvas.drawList,
                           ImGui::GetIO().MousePos,
                           {canvas.position.x + canvas.size.x, canvas.position.y + canvas.size.y},
                           camera.pixelsPerMeter(canvas.size),
                           draggingNode,
                           draggingGeometryNode,
                           selectedGeometrySpan,
                           activeTool};
  auto reserveNodeLabelBounds = [&]()
  {
    context.world.forEachNode(
        [&](AbstractNodeId nodeId, network::AbstractNode&)
        {
          const ImVec2 position = context.projection.worldToScreen(
              context.world.getPhysicalNode(nodeId).getCoordinate());
          const float radius = nodeMarkerRadius(context.pixelsPerMeter) + 3.0f;
          context.placedLabels.emplace_back(ImVec2(position.x - radius, position.y - radius),
                                            ImVec2(position.x + radius, position.y + radius));
        });
  };

  auto updateRouteHover =
      [&](AbstractRouteId routeId, size_t spanIndex, ImVec2 start, ImVec2 end, float width)
  {
    const float deltaX = end.x - start.x;
    const float deltaY = end.y - start.y;
    const float lengthSquared = deltaX * deltaX + deltaY * deltaY;
    if (lengthSquared <= 0.0f)
    {
      return;
    }

    const float factor =
        std::clamp(((context.mouse.x - start.x) * deltaX + (context.mouse.y - start.y) * deltaY) /
                       lengthSquared,
                   0.0f, 1.0f);
    const float closestX = start.x + factor * deltaX;
    const float closestY = start.y + factor * deltaY;
    const float offsetX = context.mouse.x - closestX;
    const float offsetY = context.mouse.y - closestY;
    if (offsetX * offsetX + offsetY * offsetY > (width + 6.0f) * (width + 6.0f))
    {
      return;
    }

    context.hovered.route = routeId;
    if (context.activeTool == Tool::Geometry && context.world.getActiveRoute().has_value() &&
        sameRoute(*context.world.getActiveRoute(), routeId))
    {
      context.hovered.geometrySpan = {.route = routeId, .index = spanIndex};
    }
  };

  auto drawRoutes = [&]()
  {
    context.world.forEachRoute(
        [&](AbstractRouteId routeId, network::AbstractRoute& route)
        {
          const network::PhysicalRouteGeometry* geometry = context.world.getRouteGeometry(routeId);
          if (geometry == nullptr)
          {
            return;
          }

          const std::vector<network::RouteGeometrySample> samples =
              network::sampleRouteGeometry(*geometry);
          if (samples.size() < 2)
          {
            return;
          }

          const network::RouteColor& color = route.getColor();
          const ImU32 routeColor =
              IM_COL32(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
                       static_cast<int>(color.b * 255), 235);
          const bool selected = context.world.getActiveRoute().has_value() &&
                                sameRoute(*context.world.getActiveRoute(), routeId);
          const float width =
              std::clamp(context.pixelsPerMeter * 0.20f, 2.5f, 6.0f) + (selected ? 2.0f : 0.0f);
          ImVec2 longestStart{};
          ImVec2 longestEnd{};
          ImVec2 centroid{};
          float longestLengthSquared = 0.0f;
          for (size_t index = 1; index < samples.size(); ++index)
          {
            const ImVec2 start = context.projection.worldToScreen(samples[index - 1].coordinate);
            const ImVec2 end = context.projection.worldToScreen(samples[index].coordinate);
            const bool geometrySpanSelected =
                context.activeTool == Tool::Geometry && context.selectedGeometrySpan.has_value() &&
                sameRoute(context.selectedGeometrySpan->route, routeId) &&
                context.selectedGeometrySpan->index == samples[index].spanIndex;
            context.drawList.AddLine(
                start, end, geometrySpanSelected ? IM_COL32(255, 205, 70, 255) : routeColor,
                width + (geometrySpanSelected ? 2.0f : 0.0f));
            context.routeSegments.emplace_back(start, end);
            updateRouteHover(routeId, samples[index].spanIndex, start, end, width);
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
            for (const network::RouteGeometrySample& sample : samples)
            {
              const ImVec2 point = context.projection.worldToScreen(sample.coordinate);
              centroid.x += point.x;
              centroid.y += point.y;
            }
            centroid.x /= static_cast<float>(samples.size());
            centroid.y /= static_cast<float>(samples.size());
            const float length = std::sqrt(longestLengthSquared);
            ImVec2 direction{(longestEnd.x - longestStart.x) / length,
                             (longestEnd.y - longestStart.y) / length};
            ImVec2 normal{-direction.y, direction.x};
            const ImVec2 midpoint{(longestStart.x + longestEnd.x) * 0.5f,
                                  (longestStart.y + longestEnd.y) * 0.5f};
            if ((centroid.x - midpoint.x) * normal.x + (centroid.y - midpoint.y) * normal.y < 0.0f)
            {
              normal = {-normal.x, -normal.y};
            }
            if (direction.x < 0.0f)
            {
              direction = {-direction.x, -direction.y};
            }
            drawRotatedRouteLabel(
                context.drawList,
                {midpoint.x + normal.x * (width + 8.0f), midpoint.y + normal.y * (width + 8.0f)},
                direction, IM_COL32(20, 30, 40, 255), route.getName().c_str());
          }

          if (context.activeTool == Tool::Geometry && selected)
          {
            for (size_t index = 0; index < geometry->nodes.size(); ++index)
            {
              const network::PhysicalRouteGeometryNode& node = geometry->nodes[index];
              const ImVec2 position = context.projection.worldToScreen(node.coordinate);
              const bool anchor = node.anchorNode.has_value();
              const bool dragging = context.draggingGeometryNode.has_value() &&
                                    sameRoute(context.draggingGeometryNode->route, routeId) &&
                                    context.draggingGeometryNode->index == index;
              const float radius = anchor ? 5.0f : 6.0f;
              const ImU32 nodeColor = anchor ? IM_COL32(245, 248, 250, 255)
                                             : IM_COL32(88, 209, 255, dragging ? 80 : 255);
              context.drawList.AddCircleFilled(position, radius, nodeColor, 12);
              context.drawList.AddCircle(position, radius + 1.0f, IM_COL32(20, 30, 40, 255), 12,
                                         1.5f);
              const float deltaX = context.mouse.x - position.x;
              const float deltaY = context.mouse.y - position.y;
              if (deltaX * deltaX + deltaY * deltaY <= (radius + 5.0f) * (radius + 5.0f))
              {
                context.hovered.geometryNode = {
                    .route = routeId, .index = index, .isAnchor = anchor};
              }
            }
          }
        });
  };

  auto drawVehicles = [&]()
  {
    context.world.forEachVehicle(
        [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
        {
          const auto pose = context.world.getVehiclePose(vehicleId);
          if (!pose.has_value())
          {
            return;
          }

          const ImVec2 position = context.projection.worldToScreen(pose->coordinate);
          const float halfLength = std::clamp(context.pixelsPerMeter * 1.1f, 9.0f, 34.0f);
          const float halfWidth = std::clamp(context.pixelsPerMeter * 0.45f, 5.0f, 14.0f);
          if (context.projection.isElevationView())
          {
            drawVehiclePrism(context.drawList, context.projection, *pose);
          }
          else
          {
            ImVec2 direction = context.projection.worldVectorToScreen(pose->direction);
            const float directionLength =
                std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (directionLength > 0.0f)
            {
              direction.x /= directionLength;
              direction.y /= directionLength;
            }
            const ImVec2 normal(-direction.y, direction.x);
            const ImVec2 frontLeft(position.x + direction.x * halfLength + normal.x * halfWidth,
                                   position.y + direction.y * halfLength + normal.y * halfWidth);
            const ImVec2 frontRight(position.x + direction.x * halfLength - normal.x * halfWidth,
                                    position.y + direction.y * halfLength - normal.y * halfWidth);
            const ImVec2 rearRight(position.x - direction.x * halfLength - normal.x * halfWidth,
                                   position.y - direction.y * halfLength - normal.y * halfWidth);
            const ImVec2 rearLeft(position.x - direction.x * halfLength + normal.x * halfWidth,
                                  position.y - direction.y * halfLength + normal.y * halfWidth);
            context.drawList.AddQuadFilled(frontLeft, frontRight, rearRight, rearLeft,
                                           IM_COL32(32, 61, 92, 255));
            context.drawList.AddQuad(frontLeft, frontRight, rearRight, rearLeft,
                                     IM_COL32(245, 248, 250, 255), 1.5f);
            context.drawList.AddLine(frontLeft, frontRight, IM_COL32(90, 196, 232, 255), 2.0f);
          }

          const float radius = std::max(halfLength, halfWidth);
          const float deltaX = context.mouse.x - position.x;
          const float deltaY = context.mouse.y - position.y;
          if (deltaX * deltaX + deltaY * deltaY <= radius * radius)
          {
            context.hovered.vehicle = vehicleId;
          }

          const std::string label =
              ui::shouldShowVehicleSpeeds()
                  ? vehicle.getDisplayName() + " (" +
                        std::to_string(static_cast<int>(std::round(vehicle.getCurrentSpeedKph()))) +
                        " km/h)"
                  : vehicle.getDisplayName();
          const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
          const std::optional<ImVec2> labelPosition = chooseClearLabelPosition(
              position, textSize, radius + 8.0f, context.canvas.position, context.canvasMaximum,
              context.routeSegments, context.placedLabels);
          if (labelPosition.has_value())
          {
            drawReadableLabel(context.drawList, *labelPosition, label.c_str());
            context.placedLabels.push_back(readableLabelBounds(*labelPosition, textSize));
          }
        });
  };

  auto drawNodes = [&]()
  {
    context.world.forEachNode(
        [&](AbstractNodeId nodeId, network::AbstractNode& node)
        {
          const network::PhysicalNode& physicalNode = context.world.getPhysicalNode(nodeId);
          const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();
          const ImVec2 position = context.projection.worldToScreen(coordinate);
          const bool selected = context.world.getActiveNode().has_value() &&
                                context.world.getActiveNode()->id == nodeId.id &&
                                context.world.getActiveNode()->generation == nodeId.generation;
          const bool selectedForRoute = ui::isRouteNodeSelected(nodeId);
          const bool dragging = context.draggingNode.has_value() &&
                                context.draggingNode->id == nodeId.id &&
                                context.draggingNode->generation == nodeId.generation;
          const float radius = nodeMarkerRadius(context.pixelsPerMeter);
          if (context.projection.isElevationView() && coordinate.z != 0.0)
          {
            const ImVec2 ground =
                context.projection.worldToScreen({coordinate.x, coordinate.y, 0.0});
            context.drawList.AddLine(ground, position, IM_COL32(76, 88, 101, 155), 1.5f);
            context.drawList.AddCircleFilled(ground, radius * 0.45f, IM_COL32(76, 88, 101, 100),
                                             12);
          }
          const ImU32 nodeColor = selectedForRoute ? IM_COL32(170, 48, 56, dragging ? 80 : 255)
                                  : selected       ? IM_COL32(190, 55, 63, dragging ? 80 : 255)
                                                   : IM_COL32(112, 116, 120, dragging ? 80 : 230);
          context.drawList.AddCircleFilled(position, radius, nodeColor, 20);
          if (selected)
          {
            context.drawList.AddCircle(position, radius + 4.0f, IM_COL32(255, 205, 70, 255), 20,
                                       2.0f);
          }
          if (selectedForRoute)
          {
            context.drawList.AddCircle(position, radius + 4.0f, IM_COL32(115, 205, 255, 255), 20,
                                       2.0f);
          }

          const float deltaX = context.mouse.x - position.x;
          const float deltaY = context.mouse.y - position.y;
          if (deltaX * deltaX + deltaY * deltaY <= 100.0f)
          {
            context.hovered.node = nodeId;
          }

          const std::string label =
              node.getName() + " (" + network::nodeTypeName(physicalNode.getNodeType()) + ")";
          const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
          const std::optional<ImVec2> labelPosition = chooseClearLabelPosition(
              position, textSize, 16.0f, context.canvas.position, context.canvasMaximum,
              context.routeSegments, context.placedLabels);
          if (labelPosition.has_value())
          {
            drawReadableLabel(context.drawList, *labelPosition, label.c_str());
            context.placedLabels.push_back(readableLabelBounds(*labelPosition, textSize));
          }
        });
  };

  auto drawHoverTooltip = [&]()
  {
    if (context.hovered.node.has_value())
    {
      const network::AbstractNode& node = context.world.getNode(*context.hovered.node);
      const network::PhysicalNode& physicalNode =
          context.world.getPhysicalNode(*context.hovered.node);
      const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();
      ImGui::SetTooltip("%s\nType: %s\nCoordinate: (%.0f, %.0f, %.0f) m", node.getName().c_str(),
                        network::nodeTypeName(physicalNode.getNodeType()), coordinate.x,
                        coordinate.y, coordinate.z);
      return;
    }

    if (!context.hovered.vehicle.has_value())
    {
      return;
    }

    context.world.forEachVehicle(
        [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
        {
          if (vehicleId.id == context.hovered.vehicle->id &&
              vehicleId.generation == context.hovered.vehicle->generation)
          {
            ImGui::SetTooltip("%s\nCurrent speed: %.1f km/h", vehicle.getDisplayName().c_str(),
                              vehicle.getCurrentSpeedKph());
          }
        });
  };

  reserveNodeLabelBounds();
  context.drawList.PushClipRect(canvas.position, context.canvasMaximum, true);
  drawRoutes();
  drawVehicles();
  drawNodes();
  context.drawList.PopClipRect();
  drawHoverTooltip();
  return context.hovered;
}
} // namespace render::grid
