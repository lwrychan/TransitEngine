#include "render/PhysicalGridView.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <imgui.h>

#include "core/CoreConfig.hpp"
#include "network/PhysicalNetwork.hpp"

namespace
{
    struct GridViewState
    {
        bool initialized = false;
        float visibleExtentMeters = 10.0f;
        float defaultExtentMeters = 10.0f;
        ImVec2 pan{0.0f, 0.0f};
    };

    GridViewState& getGridViewState() {
        static GridViewState state;
        return state;
    }

    void applyGridPanelLayout() {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        constexpr float margin = 10.0f;
        constexpr float panelGap = 10.0f;
        const float sideWidth = std::clamp(viewport->WorkSize.x * 0.24f, 260.0f, 320.0f);
        const float gridX = viewport->WorkPos.x + margin + sideWidth + panelGap;
        const float rightX = viewport->WorkPos.x + viewport->WorkSize.x - margin - sideWidth;

        ImGui::SetNextWindowPos(ImVec2(gridX, viewport->WorkPos.y + margin), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(
            std::max(150.0f, rightX - panelGap - gridX),
            std::max(180.0f, viewport->WorkSize.y - margin * 2.0f)), ImGuiCond_Always);
    }

    float areaToSquareSide(double areaSquareMeters) {
        return static_cast<float>(std::sqrt(std::max(1.0, areaSquareMeters)));
    }
}

void render::drawPhysicalGridView(
    const network::PhysicalNetwork& physicalNetwork,
    const RenderingConfig& config) {
    applyGridPanelLayout();
    ImGui::Begin("Physical Grid", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    const float minimumExtentMeters = areaToSquareSide(config.minimumGridZoomAreaSquareMeters);
    const float maximumExtentMeters = std::max(
        minimumExtentMeters, areaToSquareSide(config.maximumGridZoomAreaSquareMeters));
    const float configuredDefaultExtent = std::clamp(
        areaToSquareSide(config.defaultGridZoomAreaSquareMeters),
        minimumExtentMeters, maximumExtentMeters);

    GridViewState& state = getGridViewState();
    if (!state.initialized) {
        state.initialized = true;
        state.visibleExtentMeters = configuredDefaultExtent;
        state.defaultExtentMeters = configuredDefaultExtent;
    }
    state.visibleExtentMeters = std::clamp(
        state.visibleExtentMeters, minimumExtentMeters, maximumExtentMeters);

    ImGui::TextDisabled("Render grid • %zu physical segment geometries",
        physicalNetwork.getSegmentGeometries().size());
    ImGui::SameLine();
    if (ImGui::SmallButton("Reset View")) {
        state.visibleExtentMeters = state.defaultExtentMeters;
        state.pan = ImVec2(0.0f, 0.0f);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Set as Default Map Zoom")) {
        state.defaultExtentMeters = state.visibleExtentMeters;
    }

    float visibleExtentMeters = state.visibleExtentMeters;
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::InputFloat("Visible Square", &visibleExtentMeters, 0.0f, 0.0f, "%.1f m")) {
        state.visibleExtentMeters = std::clamp(
            visibleExtentMeters, minimumExtentMeters, maximumExtentMeters);
    }
    ImGui::SameLine();
    if (ImGui::BeginCombo("##GridExtentPresets", "Extent Presets")) {
        constexpr float presets[] = {5.0f, 10.0f, 25.0f, 50.0f, 100.0f, 250.0f, 500.0f};
        for (float preset : presets) {
            if (preset < minimumExtentMeters || preset > maximumExtentMeters) {
                continue;
            }
            char label[32];
            std::snprintf(label, sizeof(label), "%.0f m x %.0f m", preset, preset);
            if (ImGui::Selectable(label, std::abs(state.visibleExtentMeters - preset) < 0.01f)) {
                state.visibleExtentMeters = preset;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled("Viewing %.1f m x %.1f m • allowed: %.1f m² to %.1f m²",
        state.visibleExtentMeters, state.visibleExtentMeters,
        config.minimumGridZoomAreaSquareMeters, config.maximumGridZoomAreaSquareMeters);
    ImGui::Separator();

    const ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float limitingCanvasSize = std::max(1.0f, std::min(available.x, available.y));
    const float pixelsPerMeter = limitingCanvasSize / state.visibleExtentMeters;

    // The grid is widened for the canvas aspect ratio so max zoom-out never reveals empty space.
    const float gridExtentMeters = maximumExtentMeters *
        std::max(available.x, available.y) / limitingCanvasSize;
    const ImVec2 gridSize(gridExtentMeters * pixelsPerMeter, gridExtentMeters * pixelsPerMeter);
    const ImVec2 centeredGridOrigin(
        canvasPosition.x + (available.x - gridSize.x) * 0.5f,
        canvasPosition.y + (available.y - gridSize.y) * 0.5f);
    const float panLimitX = std::max(0.0f, (gridSize.x - available.x) * 0.5f);
    const float panLimitY = std::max(0.0f, (gridSize.y - available.y) * 0.5f);
    state.pan.x = std::clamp(state.pan.x, -panLimitX, panLimitX);
    state.pan.y = std::clamp(state.pan.y, -panLimitY, panLimitY);
    const ImVec2 gridOrigin(centeredGridOrigin.x + state.pan.x, centeredGridOrigin.y + state.pan.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(canvasPosition,
        ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y), true);
    drawList->AddRectFilled(canvasPosition,
        ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y), IM_COL32(246, 248, 250, 255));
    drawList->AddRectFilled(gridOrigin,
        ImVec2(gridOrigin.x + gridSize.x, gridOrigin.y + gridSize.y), IM_COL32(255, 255, 255, 255));

    const int gridLineCount = static_cast<int>(std::ceil(gridExtentMeters));
    const int minorStep = pixelsPerMeter >= 5.0f ? 1 : (pixelsPerMeter >= 0.8f ? 5 : 25);
    const float minorFade = std::clamp((pixelsPerMeter - 0.8f) / 4.2f, 0.0f, 1.0f);
    const float majorFade = std::clamp((pixelsPerMeter * 25.0f - 2.0f) / 18.0f, 0.0f, 1.0f);
    const ImU32 minorGridColour = IM_COL32(142, 157, 172, static_cast<int>(34.0f * minorFade));
    const ImU32 majorGridColour = IM_COL32(112, 128, 145, static_cast<int>(20.0f + 78.0f * majorFade));
    for (int x = 0; x <= gridLineCount; ++x) {
        if (x % minorStep == 0) {
            const float position = gridOrigin.x + pixelsPerMeter * x;
            drawList->AddLine(ImVec2(position, gridOrigin.y), ImVec2(position, gridOrigin.y + gridSize.y),
                x % 25 == 0 ? majorGridColour : minorGridColour);
        }
    }
    for (int y = 0; y <= gridLineCount; ++y) {
        if (y % minorStep == 0) {
            const float position = gridOrigin.y + pixelsPerMeter * y;
            drawList->AddLine(ImVec2(gridOrigin.x, position), ImVec2(gridOrigin.x + gridSize.x, position),
                y % 25 == 0 ? majorGridColour : minorGridColour);
        }
    }
    drawList->PopClipRect();

    ImGui::InvisibleButton("PhysicalGridCanvas", available);
    const bool canvasHovered = ImGui::IsMouseHoveringRect(canvasPosition,
        ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y), true);
    if (canvasHovered) {
        ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        if (ImGui::GetIO().MouseWheel != 0.0f) {
            const float nextExtent = std::clamp(
                state.visibleExtentMeters / std::pow(1.15f, ImGui::GetIO().MouseWheel),
                minimumExtentMeters, maximumExtentMeters);
            const float nextPixelsPerMeter = limitingCanvasSize / nextExtent;
            const ImVec2 nextGridSize(gridExtentMeters * nextPixelsPerMeter,
                gridExtentMeters * nextPixelsPerMeter);
            const ImVec2 nextCenteredOrigin(canvasPosition.x + (available.x - nextGridSize.x) * 0.5f,
                canvasPosition.y + (available.y - nextGridSize.y) * 0.5f);
            const ImVec2 mouseWorld((mouse.x - gridOrigin.x) / pixelsPerMeter,
                (mouse.y - gridOrigin.y) / pixelsPerMeter);
            state.pan = ImVec2(mouse.x - nextCenteredOrigin.x - mouseWorld.x * nextPixelsPerMeter,
                mouse.y - nextCenteredOrigin.y - mouseWorld.y * nextPixelsPerMeter);
            state.visibleExtentMeters = nextExtent;
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            state.pan.x = std::clamp(state.pan.x + ImGui::GetIO().MouseDelta.x, -panLimitX, panLimitX);
            state.pan.y = std::clamp(state.pan.y + ImGui::GetIO().MouseDelta.y, -panLimitY, panLimitY);
        }

        const int pointX = static_cast<int>(std::round((mouse.x - gridOrigin.x) / pixelsPerMeter));
        const int pointY = static_cast<int>(std::round((mouse.y - gridOrigin.y) / pixelsPerMeter));
        if (pointX >= 0 && pointX <= gridLineCount && pointY >= 0 && pointY <= gridLineCount) {
            drawList->PushClipRect(canvasPosition,
                ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y), true);
            drawList->AddCircleFilled(ImVec2(gridOrigin.x + pointX * pixelsPerMeter,
                gridOrigin.y + pointY * pixelsPerMeter),
                std::clamp(pixelsPerMeter * 0.18f, 1.5f, 7.0f), IM_COL32(0, 0, 0, 105));
            drawList->PopClipRect();
            ImGui::SetTooltip("Snapped point (%d, %d) m\nScroll to zoom • middle-drag to pan", pointX, pointY);
        }
    }

    ImGui::End();
}
