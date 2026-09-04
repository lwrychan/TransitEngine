#pragma once

#include "render/GridTool.hpp"
#include "render/grid/GridCamera.hpp"
#include "render/grid/GridInteraction.hpp"
#include "render/grid/GridScene.hpp"
#include "render/grid/GridViewMode.hpp"

namespace core
{
class World;
}

namespace render::grid
{
class PhysicalGridViewport
{
  public:
  GridCamera& getCamera();
  const GridInteraction& getInteraction() const;
  void draw(core::World& world, Tool activeTool, GridViewMode viewMode, bool modalOpen);

  private:
  GridCamera camera;
  GridScene scene;
  GridInteraction interaction;
};
} // namespace render::grid
