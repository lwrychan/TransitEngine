#pragma once

#include <optional>
#include <vector>

#include <imgui.h>

#include "Identifiers.hpp"
#include "render/GridTool.hpp"
#include "render/grid/GridViewMode.hpp"

namespace core
{
class World;
}
namespace render::grid
{
class GridCamera;
}

namespace render::grid
{
struct HoverTargets
{
  std::optional<AbstractNodeId> node;
  std::optional<AbstractRouteId> route;
  std::optional<VehicleId> vehicle;
  struct GeometryNode
  {
    AbstractRouteId route;
    size_t index = 0;
    bool isAnchor = false;
  };
  struct GeometrySpan
  {
    AbstractRouteId route;
    size_t index = 0;
  };
  std::optional<GeometryNode> geometryNode;
  std::optional<GeometrySpan> geometrySpan;
};

struct GridCanvas
{
  ImVec2 position;
  ImVec2 size;
  ImDrawList* drawList = nullptr;
};

class GridScene
{
  public:
  HoverTargets draw(core::World& world, const GridCamera& camera, const GridCanvas& canvas,
                    const std::optional<AbstractNodeId>& draggingNode,
                    const std::optional<HoverTargets::GeometryNode>& draggingGeometryNode,
                    const std::optional<HoverTargets::GeometrySpan>& selectedGeometrySpan,
                    GridViewMode viewMode, Tool activeTool);
};
} // namespace render::grid
