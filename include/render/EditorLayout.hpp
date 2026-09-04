#pragma once

#include <imgui.h>

namespace render::editorlayout
{
inline constexpr float outerMargin = 2.0f;
inline constexpr float panelGap = 2.0f;
inline constexpr float toolBarHeight = 40.0f;
inline constexpr float toolBarTopGap = 2.0f;
inline constexpr float routeControlsHeight = 42.0f;
inline constexpr float bottomControlsGap = panelGap;

struct ContentBounds
{
  float left;
  float right;
  float top;
  float bottom;
};

inline ContentBounds contentBounds(const ImGuiViewport& viewport, float menuBarHeight)
{
  return {.left = viewport.WorkPos.x + outerMargin,
          .right = viewport.WorkPos.x + viewport.WorkSize.x - outerMargin,
          .top = viewport.Pos.y + menuBarHeight + toolBarTopGap + toolBarHeight + outerMargin,
          .bottom =
              viewport.WorkPos.y + viewport.WorkSize.y - routeControlsHeight - bottomControlsGap};
}
} // namespace render::editorlayout
