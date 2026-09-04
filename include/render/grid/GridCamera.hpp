#pragma once

#include <imgui.h>

#include "network/PhysicalCoordinate.hpp"

namespace render::grid
{
class GridCamera
{
  public:
  void setExtent(float extentMeters, float minimumExtent, float maximumExtent);
  float getExtent() const;
  void panBy(ImVec2 deltaPixels);
  void resetPan();
  void centerOn(const network::PhysicalCoordinate& world, ImVec2 canvasPosition, ImVec2 canvasSize);
  void saveHome();
  void restoreHome();
  void zoomAt(ImVec2 screenPoint, ImVec2 canvasPosition, ImVec2 canvasSize, float wheelDelta,
              float minimumExtent, float maximumExtent);
  ImVec2 origin(ImVec2 canvasPosition, ImVec2 canvasSize) const;
  float pixelsPerMeter(ImVec2 canvasSize) const;
  network::PhysicalCoordinate screenToWorld(ImVec2 screen, ImVec2 canvasPosition,
                                            ImVec2 canvasSize) const;
  ImVec2 worldToScreen(const network::PhysicalCoordinate& world, ImVec2 canvasPosition,
                       ImVec2 canvasSize) const;
  network::PhysicalCoordinate snapToMeter(const network::PhysicalCoordinate& world) const;

  private:
  float extentMeters = 10.0f;
  ImVec2 panPixels{0.0f, 0.0f};
  float homeExtentMeters = 10.0f;
  ImVec2 homePanPixels{0.0f, 0.0f};
};
} // namespace render::grid
