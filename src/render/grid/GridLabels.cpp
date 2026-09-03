#include "render/grid/GridLabels.hpp"

#include <algorithm>
#include <cmath>

namespace render::grid
{
void drawReadableLabel(ImDrawList& drawList, ImVec2 position, const char* text)
{
    const ImVec2 size = ImGui::CalcTextSize(text);
    drawList.AddRectFilled({position.x - 3.0f, position.y - 2.0f},
                           {position.x + size.x + 3.0f, position.y + size.y + 2.0f},
                           IM_COL32(20, 30, 40, 185), 3.0f);
    drawList.AddRect({position.x - 3.0f, position.y - 2.0f},
                     {position.x + size.x + 3.0f, position.y + size.y + 2.0f},
                     IM_COL32(230, 235, 240, 190), 3.0f);
    drawList.AddText(position, IM_COL32(255, 255, 255, 255), text);
}

ImVec2 chooseClearLabelPosition(ImVec2 anchor, ImVec2 size, float clearance, ImVec2 minimum,
                                ImVec2 maximum, const std::vector<std::pair<ImVec2, ImVec2>>&,
                                const std::vector<ImRect>& placed)
{
    constexpr ImVec2 directions[] = {{1, 0},  {1, -1}, {0, -1}, {-1, -1},
                                     {-1, 0}, {-1, 1}, {0, 1},  {1, 1}};
    for (ImVec2 direction : directions)
    {
        const float length = std::hypot(direction.x, direction.y);
        ImVec2 position(anchor.x + direction.x / length * clearance,
                        anchor.y + direction.y / length * clearance);
        position.x = std::clamp(position.x, minimum.x, maximum.x - size.x);
        position.y = std::clamp(position.y, minimum.y, maximum.y - size.y);
        const ImRect candidate(position, {position.x + size.x, position.y + size.y});
        if (std::none_of(placed.begin(), placed.end(),
                         [&](const ImRect& label) { return candidate.Overlaps(label); }))
            return position;
    }
    return anchor;
}

void drawRotatedRouteLabel(ImDrawList& drawList, ImVec2 center, ImVec2 direction, ImU32 color,
                           const char* text)
{
    const ImVec2 size = ImGui::CalcTextSize(text);
    const int firstVertex = drawList.VtxBuffer.Size;
    drawList.AddText({center.x - size.x * 0.5f, center.y - size.y * 0.5f}, color, text);
    const float angle = std::atan2(direction.y, direction.x);
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    for (int index = firstVertex; index < drawList.VtxBuffer.Size; ++index)
    {
        ImDrawVert& vertex = drawList.VtxBuffer[index];
        const float x = vertex.pos.x - center.x;
        const float y = vertex.pos.y - center.y;
        vertex.pos = {center.x + x * cosine - y * sine, center.y + x * sine + y * cosine};
    }
}
} // namespace render::grid
