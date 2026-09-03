#pragma once

#include <optional>
#include <vector>

#include <imgui.h>

#include "Identifiers.hpp"

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
                      const std::optional<AbstractNodeId>& draggingNode);
};
} // namespace render::grid
