#pragma once

#include "render/GridTool.hpp"
#include "render/grid/GridViewMode.hpp"

namespace core
{
class World;
}
struct RenderingConfig;

namespace render::grid
{
void drawPhysicalGridView(core::World& world, const RenderingConfig& config, Tool activeTool,
                          GridViewMode viewMode, bool modalOpen, bool showDebugPanels);
}
