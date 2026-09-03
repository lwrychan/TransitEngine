#pragma once

#include "render/GridTool.hpp"

namespace core
{
class World;
}
struct RenderingConfig;

namespace render::grid
{
void drawPhysicalGridView(core::World& world, const RenderingConfig& config, Tool activeTool,
                          bool modalOpen);
}
