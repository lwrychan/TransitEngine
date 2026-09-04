#include "render/grid/GridPanelLayout.hpp"

#include <algorithm>

#include <imgui.h>

#include "render/EditorLayout.hpp"

namespace render::grid
{
void applyPhysicalGridPanelLayout(bool showDebugPanels)
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const editorlayout::ContentBounds bounds =
      editorlayout::contentBounds(*viewport, ImGui::GetFrameHeight());
  const float sideWidth = std::clamp(viewport->WorkSize.x * 0.24f, 260.0f, 320.0f);
  const float gridX = bounds.left + sideWidth + editorlayout::panelGap;
  const float rightX = showDebugPanels ? bounds.right - sideWidth : bounds.right;
  const float gridTop = bounds.top;
  const float gridBottom = bounds.bottom;
  ImGui::SetNextWindowPos({gridX, gridTop}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({std::max(1.0f, rightX - editorlayout::panelGap - gridX),
                            std::max(1.0f, gridBottom - gridTop)},
                           ImGuiCond_Always);
}
} // namespace render::grid
