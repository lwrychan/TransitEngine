#include "render/grid/GridGeometryControls.hpp"

#include <cmath>

#include <imgui.h>

#include "core/World.hpp"
#include "ui/UIStyle.hpp"

namespace render::grid
{
namespace
{
  std::optional<HoverTargets::GeometrySpan> selectedGeometrySpan;
} // namespace

const std::optional<HoverTargets::GeometrySpan>& getSelectedGeometrySpan()
{
  return selectedGeometrySpan;
}

void selectGeometrySpan(HoverTargets::GeometrySpan span)
{
  selectedGeometrySpan = span;
}

void clearSelectedGeometrySpan()
{
  selectedGeometrySpan.reset();
}

void drawGridGeometryControls(core::World& world)
{
  const std::optional<HoverTargets::GeometrySpan>& selected = getSelectedGeometrySpan();
  if (!selected.has_value())
  {
    return;
  }

  const network::PhysicalRouteGeometry* geometry = world.getRouteGeometry(selected->route);
  if (geometry == nullptr || selected->index >= geometry->spans.size())
  {
    return;
  }

  ImGui::SeparatorText("Geometry Span");
  const network::PhysicalRouteGeometrySpan& span = geometry->spans[selected->index];
  bool bezier = span.interpolation == network::GeometryInterpolation::Bezier;
  ImGui::Text("Span %zu", selected->index + 1);
  if (ImGui::Checkbox("Bezier", &bezier))
  {
    world.setRouteGeometrySpanInterpolation(selected->route, selected->index,
                                            bezier ? network::GeometryInterpolation::Bezier
                                                   : network::GeometryInterpolation::Linear);
  }
  double speedLimitKph = span.maximumSpeedKph;
  ImGui::SetNextItemWidth(160.0f);
  ui::pushTextInputStyle();
  if (ImGui::InputDouble("Max speed (km/h)", &speedLimitKph, 1.0, 10.0, "%.1f") &&
      speedLimitKph > 0.0 && std::isfinite(speedLimitKph))
  {
    world.setRouteGeometrySpanSpeedLimit(selected->route, selected->index, speedLimitKph);
  }
  ui::popTextInputStyle();
}
} // namespace render::grid
