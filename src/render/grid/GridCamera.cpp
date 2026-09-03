#include "render/grid/GridCamera.hpp"

#include <algorithm>
#include <cmath>

namespace render::grid
{
void GridCamera::setExtent(float extent, float minimum, float maximum)
{
    extentMeters = std::clamp(extent, minimum, maximum);
}
float GridCamera::getExtent() const
{
    return extentMeters;
}
void GridCamera::panBy(ImVec2 delta)
{
    panPixels.x += delta.x;
    panPixels.y += delta.y;
}
void GridCamera::resetPan()
{
    panPixels = {0.0f, 0.0f};
}
void GridCamera::centerOn(const network::PhysicalCoordinate& world, ImVec2, ImVec2 size)
{
    const float scale = pixelsPerMeter(size);
    panPixels = {-static_cast<float>(world.x) * scale, static_cast<float>(world.y) * scale};
}
void GridCamera::saveHome()
{
    homeExtentMeters = extentMeters;
    homePanPixels = panPixels;
}
void GridCamera::restoreHome()
{
    extentMeters = homeExtentMeters;
    panPixels = homePanPixels;
}
void GridCamera::zoomAt(ImVec2 screen, ImVec2 position, ImVec2 size, float wheel, float minimum,
                        float maximum)
{
    const network::PhysicalCoordinate anchoredWorld = screenToWorld(screen, position, size);
    extentMeters = std::clamp(extentMeters / std::pow(1.15f, wheel), minimum, maximum);
    const float scale = pixelsPerMeter(size);
    panPixels = {
        screen.x - position.x - size.x * 0.5f - static_cast<float>(anchoredWorld.x) * scale,
        screen.y - position.y - size.y * 0.5f + static_cast<float>(anchoredWorld.y) * scale};
}
ImVec2 GridCamera::origin(ImVec2 position, ImVec2 size) const
{
    return {position.x + size.x * 0.5f + panPixels.x, position.y + size.y * 0.5f + panPixels.y};
}
float GridCamera::pixelsPerMeter(ImVec2 size) const
{
    return std::max(1.0f, std::min(size.x, size.y)) / extentMeters;
}
network::PhysicalCoordinate GridCamera::screenToWorld(ImVec2 screen, ImVec2 position,
                                                      ImVec2 size) const
{
    const ImVec2 center = origin(position, size);
    const float scale = pixelsPerMeter(size);
    return {(screen.x - center.x) / scale, (center.y - screen.y) / scale, 0.0};
}
ImVec2 GridCamera::worldToScreen(const network::PhysicalCoordinate& world, ImVec2 position,
                                 ImVec2 size) const
{
    const ImVec2 center = origin(position, size);
    const float scale = pixelsPerMeter(size);
    return {center.x + static_cast<float>(world.x) * scale,
            center.y - static_cast<float>(world.y) * scale};
}
network::PhysicalCoordinate GridCamera::snapToMeter(const network::PhysicalCoordinate& world) const
{
    return {std::round(world.x), std::round(world.y), std::round(world.z)};
}
} // namespace render::grid
