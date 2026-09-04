#pragma once

#include <optional>
#include <utility>
#include <vector>

#include <imgui.h>
#include <imgui_internal.h>

namespace render::grid
{
ImRect readableLabelBounds(ImVec2 position, ImVec2 textSize);
void drawReadableLabel(ImDrawList& drawList, ImVec2 position, const char* text);
void drawRotatedRouteLabel(ImDrawList& drawList, ImVec2 center, ImVec2 direction, ImU32 color,
                           const char* text);
std::optional<ImVec2> chooseClearLabelPosition(
    ImVec2 anchor, ImVec2 textSize, float clearance, ImVec2 canvasMinimum, ImVec2 canvasMaximum,
    const std::vector<std::pair<ImVec2, ImVec2>>& routeSegments,
    const std::vector<ImRect>& placedLabels);
} // namespace render::grid
