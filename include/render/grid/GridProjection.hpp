#pragma once

#include <imgui.h>

#include "network/PhysicalCoordinate.hpp"
#include "render/grid/GridViewMode.hpp"

namespace render::grid
{
class GridCamera;

// Shared plan/axonometric transform so the backdrop and scene cannot drift apart.
class GridProjection
{
  public:
  GridProjection(const GridCamera& camera, ImVec2 canvasPosition, ImVec2 canvasSize,
                 GridViewMode viewMode);

  ImVec2 worldToScreen(const network::PhysicalCoordinate& coordinate) const;
  ImVec2 worldVectorToScreen(const network::PhysicalCoordinate& vector) const;
  bool isElevationView() const;

  private:
  const GridCamera& camera;
  ImVec2 canvasPosition;
  ImVec2 canvasSize;
  GridViewMode viewMode;
};
} // namespace render::grid
