#pragma once

#include <optional>

#include "render/grid/GridScene.hpp"

namespace core
{
class World;
}
namespace render
{
enum class Tool;
}
namespace render::grid
{
class GridCamera;
}

namespace render::grid
{
class GridInteraction
{
  public:
  void handle(core::World& world, GridCamera& camera, const GridCanvas& canvas,
              const HoverTargets& hoverTargets, Tool activeTool, bool editingEnabled,
              bool modalOpen);
  const std::optional<AbstractNodeId>& getDraggingNode() const
  {
    return draggingNode;
  }
  const std::optional<HoverTargets::GeometryNode>& getDraggingGeometryNode() const
  {
    return draggingGeometryNode;
  }

  private:
  std::optional<AbstractNodeId> draggingNode;
  std::optional<HoverTargets::GeometryNode> draggingGeometryNode;
  ImVec2 dragStart{};
};
} // namespace render::grid
