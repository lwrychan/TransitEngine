#pragma once

namespace render::grid
{
enum class GridViewMode;
class GridCamera;
struct GridCanvas;
} // namespace render::grid

namespace render::grid
{
void drawGridBackdrop(const GridCamera& camera, const GridCanvas& canvas, GridViewMode viewMode);
}
