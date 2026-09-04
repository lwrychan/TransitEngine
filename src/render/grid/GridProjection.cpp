#include "render/grid/GridProjection.hpp"

#include "render/grid/GridCamera.hpp"

namespace render::grid
{
namespace
{
  constexpr float AxonometricHorizontalScale = 0.8660254f;
  constexpr float AxonometricVerticalScale = 0.5f;
  constexpr float ElevationScale = 0.85f;
} // namespace

GridProjection::GridProjection(const GridCamera& camera, ImVec2 canvasPosition, ImVec2 canvasSize,
                               GridViewMode viewMode)
    : camera(camera), canvasPosition(canvasPosition), canvasSize(canvasSize), viewMode(viewMode)
{
}

ImVec2 GridProjection::worldToScreen(const network::PhysicalCoordinate& coordinate) const
{
  if (viewMode == GridViewMode::Plan)
  {
    return camera.worldToScreen(coordinate, canvasPosition, canvasSize);
  }

  const ImVec2 origin = camera.origin(canvasPosition, canvasSize);
  const float scale = camera.pixelsPerMeter(canvasSize);
  return {origin.x +
              static_cast<float>(coordinate.x - coordinate.y) * scale * AxonometricHorizontalScale,
          origin.y +
              static_cast<float>(coordinate.x + coordinate.y) * scale * AxonometricVerticalScale -
              static_cast<float>(coordinate.z) * scale * ElevationScale};
}

ImVec2 GridProjection::worldVectorToScreen(const network::PhysicalCoordinate& vector) const
{
  const float scale = camera.pixelsPerMeter(canvasSize);
  if (viewMode == GridViewMode::Plan)
  {
    return {static_cast<float>(vector.x) * scale, -static_cast<float>(vector.y) * scale};
  }

  return {static_cast<float>(vector.x - vector.y) * scale * AxonometricHorizontalScale,
          static_cast<float>(vector.x + vector.y) * scale * AxonometricVerticalScale -
              static_cast<float>(vector.z) * scale * ElevationScale};
}

bool GridProjection::isElevationView() const
{
  return viewMode == GridViewMode::Elevation;
}
} // namespace render::grid
