#pragma once

namespace core
{
class World;
}
struct RenderingConfig;

namespace render::grid
{
class GridCamera;

void drawGridNavigationControls(core::World& world, GridCamera& camera,
                                const RenderingConfig& config);
} // namespace render::grid
