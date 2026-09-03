#pragma once

#include <imgui.h>

namespace ui
{
    inline void pushTextInputStyle()
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.78f, 0.78f, 0.78f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.73f, 0.73f, 0.73f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.68f, 0.68f, 0.68f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.64f, 0.64f, 0.64f, 1.0f));
    }

    inline void popTextInputStyle()
    {
        ImGui::PopStyleColor(4);
    }
}
