#pragma once

#include <optional>

#include "render/grid/GridScene.hpp"

namespace core
{
class World;
}

namespace render::grid
{
const std::optional<HoverTargets::GeometrySpan>& getSelectedGeometrySpan();
void selectGeometrySpan(HoverTargets::GeometrySpan span);
void clearSelectedGeometrySpan();
void drawGridGeometryControls(core::World& world);
} // namespace render::grid
