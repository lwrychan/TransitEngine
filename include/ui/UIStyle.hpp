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

inline bool button(const char* label, ImVec2 size = ImVec2(0.0f, 0.0f))
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor();
    return pressed;
}

inline bool smallButton(const char* label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    const bool pressed = ImGui::SmallButton(label);
    ImGui::PopStyleColor();
    return pressed;
}

inline bool beginTabItem(const char* label)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    const bool open = ImGui::BeginTabItem(label);
    ImGui::PopStyleColor();
    return open;
}
} // namespace ui
