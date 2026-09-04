#include "render/grid/GridBackdrop.hpp"

#include <cmath>
#include <cstdio>

#include "render/grid/GridCamera.hpp"
#include "render/grid/GridProjection.hpp"
#include "render/grid/GridScene.hpp"

namespace render::grid
{
namespace
{
  float gridStep(float pixelsPerMeter)
  {
    const float raw = 50.0f / pixelsPerMeter;
    const float magnitude = std::pow(10.0f, std::floor(std::log10(raw)));
    const float normalized = raw / magnitude;
    return std::max(1.0f, (normalized <= 1.0f   ? 1.0f
                           : normalized <= 2.0f ? 2.0f
                           : normalized <= 5.0f ? 5.0f
                                                : 10.0f) *
                              magnitude);
  }
} // namespace

void drawGridBackdrop(const GridCamera& camera, const GridCanvas& canvas, GridViewMode viewMode)
{
  if (canvas.drawList == nullptr)
    return;
  const ImVec2 maximum(canvas.position.x + canvas.size.x, canvas.position.y + canvas.size.y);
  const float pixelsPerMeter = camera.pixelsPerMeter(canvas.size);
  const float step = gridStep(pixelsPerMeter);
  const GridProjection projection(camera, canvas.position, canvas.size, viewMode);
  canvas.drawList->PushClipRect(canvas.position, maximum, true);
  canvas.drawList->AddRectFilled(canvas.position, maximum, IM_COL32(246, 248, 250, 255));
  const network::PhysicalCoordinate center = camera.screenToWorld(
      {canvas.position.x + canvas.size.x * 0.5f, canvas.position.y + canvas.size.y * 0.5f},
      canvas.position, canvas.size);
  const double range =
      projection.isElevationView() ? camera.getExtent() * 0.8 : camera.getExtent() * 0.5;
  const double minimumX = center.x - range;
  const double maximumX = center.x + range;
  const double minimumY = center.y - range;
  const double maximumY = center.y + range;
  const double majorStep = static_cast<double>(step) * 5.0;
  const double firstX = std::ceil(minimumX / step) * step;
  const double firstY = std::ceil(minimumY / step) * step;
  for (double x = firstX; x <= maximumX + step * 0.5; x += step)
  {
    const bool axis = std::abs(x) < step * 0.001;
    const bool major = std::abs(std::fmod(std::abs(x), majorStep)) < step * 0.001;
    const ImU32 color = axis    ? IM_COL32(80, 95, 110, 150)
                        : major ? IM_COL32(112, 128, 145, 85)
                                : IM_COL32(142, 157, 172, 34);
    if (projection.isElevationView())
    {
      canvas.drawList->AddLine(projection.worldToScreen({x, minimumY, 0.0}),
                               projection.worldToScreen({x, maximumY, 0.0}), color);
    }
    else
    {
      const float screenX = projection.worldToScreen({x, 0.0, 0.0}).x;
      canvas.drawList->AddLine({screenX, canvas.position.y}, {screenX, maximum.y}, color);
    }
  }
  for (double y = firstY; y <= maximumY + step * 0.5; y += step)
  {
    const bool axis = std::abs(y) < step * 0.001;
    const bool major = std::abs(std::fmod(std::abs(y), majorStep)) < step * 0.001;
    const ImU32 color = axis    ? IM_COL32(80, 95, 110, 150)
                        : major ? IM_COL32(112, 128, 145, 85)
                                : IM_COL32(142, 157, 172, 34);
    if (projection.isElevationView())
    {
      canvas.drawList->AddLine(projection.worldToScreen({minimumX, y, 0.0}),
                               projection.worldToScreen({maximumX, y, 0.0}), color);
    }
    else
    {
      const float screenY = projection.worldToScreen({0.0, y, 0.0}).y;
      canvas.drawList->AddLine({canvas.position.x, screenY}, {maximum.x, screenY}, color);
    }
  }
  const float barPixels = std::min(step * pixelsPerMeter, std::max(24.0f, canvas.size.x - 48.0f));
  const ImVec2 barStart(canvas.position.x + 16.0f, maximum.y - 18.0f);
  const ImVec2 barEnd(barStart.x + barPixels, barStart.y);
  char label[32];
  std::snprintf(label, sizeof(label), step >= 1000.0f ? "%.1f km" : "%.0f m",
                step >= 1000.0f ? step / 1000.0f : step);
  const ImVec2 labelSize = ImGui::CalcTextSize(label);
  canvas.drawList->AddRectFilled({barStart.x - 7.0f, barStart.y - labelSize.y - 12.0f},
                                 {barEnd.x + 7.0f, barStart.y + 7.0f}, IM_COL32(246, 248, 250, 225),
                                 3.0f);
  canvas.drawList->AddLine(barStart, barEnd, IM_COL32(20, 30, 40, 255), 2.0f);
  canvas.drawList->AddLine({barStart.x, barStart.y - 5.0f}, {barStart.x, barStart.y + 5.0f},
                           IM_COL32(20, 30, 40, 255), 2.0f);
  canvas.drawList->AddLine({barEnd.x, barEnd.y - 5.0f}, {barEnd.x, barEnd.y + 5.0f},
                           IM_COL32(20, 30, 40, 255), 2.0f);
  canvas.drawList->AddText({barStart.x, barStart.y - labelSize.y - 6.0f}, IM_COL32(20, 30, 40, 255),
                           label);
  canvas.drawList->PopClipRect();
}
} // namespace render::grid
