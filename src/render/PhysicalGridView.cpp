#include "render/PhysicalGridView.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include <iostream>

#include <imgui.h>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "network/PhysicalNetwork.hpp"
#include "ui/DebugPanels.hpp"
#include "ui/UIStyle.hpp"
#include "vehicle/Vehicle.hpp"

#include "network/PhysicalCoordinate.hpp"

namespace render
{
    struct GridViewState
    {
        bool initialized = false;

        // Current camera.
        float visibleExtentMeters = 10.0f;
        ImVec2 pan{ 0.0f, 0.0f };

        // Saved home camera.
        float homeExtentMeters = 10.0f;
        ImVec2 homePan{ 0.0f, 0.0f };

        std::optional<AbstractNodeId> draggingNode;
        ImVec2 dragStartScreen{ 0.0f, 0.0f };
        bool dragMoved = false;
    };

    GridViewState& getGridViewState()
    {
        static GridViewState state;
        return state;
    }

    double zeroRound(double number) {
        double roundedNumber = std::round(number);

        if (std::abs(number) < 1.0 && number < 0.0) {
            return std::abs(roundedNumber);
        }
        return roundedNumber;

    }

    void applyGridPanelLayout()
    {
        const ImGuiViewport* viewport =
            ImGui::GetMainViewport();

        constexpr float margin = 10.0f;
        constexpr float panelGap = 10.0f;

        const float sideWidth =
            std::clamp(
                viewport->WorkSize.x * 0.24f,
                260.0f,
                320.0f);

        const float gridX =
            viewport->WorkPos.x +
            margin +
            sideWidth +
            panelGap;

        constexpr float toolbarOffset = 70.0f;

        const float rightX =
            viewport->WorkPos.x +
            viewport->WorkSize.x -
            margin -
            sideWidth;

        ImGui::SetNextWindowPos(
            ImVec2(
                gridX,
                viewport->WorkPos.y + margin + toolbarOffset),
            ImGuiCond_Always);

        ImGui::SetNextWindowSize(
            ImVec2(
                std::max(
                    150.0f,
                    rightX - panelGap - gridX),

                std::max(
                    180.0f,
                    viewport->WorkSize.y -
                    margin * 2.0f - toolbarOffset)),
            ImGuiCond_Always);
    }

    float areaToSquareSide(double areaSquareMeters)
    {
        return static_cast<float>(
            std::sqrt(
                std::max(
                    1.0,
                    areaSquareMeters)));
    }


    // ============================================================
    // Physical coordinate conversion
    //
    // The rendered view is currently the XY plane:
    //
    //                  +Y
    //                  ↑
    //                  |
    //                  |
    //        ----------+----------→ +X
    //                  |
    //                  |
    //                  ↓
    //
    //                 -Y
    //
    // Z is currently fixed at 0.
    //
    // 'origin' is the screen position corresponding to:
    //
    //     PhysicalCoordinate { 0, 0, 0 }
    //
    // ============================================================

    network::PhysicalCoordinate screenToPhysical(
        ImVec2 screen,
        ImVec2 origin,
        float pixelsPerMeter)
    {
        return network::PhysicalCoordinate{
            static_cast<double>(
                screen.x - origin.x) /
                pixelsPerMeter,

            static_cast<double>(
                origin.y - screen.y) /
                pixelsPerMeter,

            0.0
        };
    }

    ImVec2 physicalToScreen(
        const network::PhysicalCoordinate& coordinate,
        ImVec2 origin,
        float pixelsPerMeter)
    {
        return ImVec2(
            origin.x +
            static_cast<float>(
                coordinate.x *
                pixelsPerMeter),

            origin.y -
            static_cast<float>(
                coordinate.y *
                pixelsPerMeter));
    }


    // ============================================================
    // Grid spacing
    //
    // Maintains approximately 50 pixels between visible grid
    // lines using a 1/2/5 × 10^n progression:
    //
    //     1
    //     2
    //     5
    //     10
    //     20
    //     50
    //     100
    //     ...
    //
    // This prevents the grid from becoming excessively dense
    // when zoomed far out.
    // ============================================================

    float chooseGridStep(float pixelsPerMeter)
    {
        constexpr float targetPixels = 50.0f;

        if (pixelsPerMeter <= 0.0f)
            return 1.0f;

        const float rawMeters =
            targetPixels /
            pixelsPerMeter;

        const float exponent =
            std::floor(
                std::log10(rawMeters));

        const float magnitude =
            std::pow(
                10.0f,
                exponent);

        const float normalized =
            rawMeters /
            magnitude;

        float niceValue;

        if (normalized <= 1.0f)
        {
            niceValue = 1.0f;
        }
        else if (normalized <= 2.0f)
        {
            niceValue = 2.0f;
        }
        else if (normalized <= 5.0f)
        {
            niceValue = 5.0f;
        }
        else
        {
            niceValue = 10.0f;
        }

        // One rendered cell is one physical metre; never subdivide below it.
        return std::max(1.0f, niceValue * magnitude);
    }

    void formatDistanceLabel(double meters, char* buffer, size_t bufferSize)
    {
        if (meters >= 1000.0) {
            std::snprintf(buffer, bufferSize, "%.1f km", meters / 1000.0);
        } else {
            std::snprintf(buffer, bufferSize, "%.0f m", meters);
        }
    }

    float distanceSquaredToSegment(ImVec2 point, ImVec2 start, ImVec2 end)
    {
        const float deltaX = end.x - start.x;
        const float deltaY = end.y - start.y;
        const float lengthSquared = deltaX * deltaX + deltaY * deltaY;
        if (lengthSquared <= 0.0f) {
            const float pointX = point.x - start.x;
            const float pointY = point.y - start.y;
            return pointX * pointX + pointY * pointY;
        }

        const float projection = std::clamp(
            ((point.x - start.x) * deltaX + (point.y - start.y) * deltaY) / lengthSquared,
            0.0f,
            1.0f);
        const float closestX = start.x + projection * deltaX;
        const float closestY = start.y + projection * deltaY;
        const float pointX = point.x - closestX;
        const float pointY = point.y - closestY;
        return pointX * pointX + pointY * pointY;
    }

    struct LabelBounds
    {
        ImVec2 minimum;
        ImVec2 maximum;
    };

    bool labelsOverlap(const LabelBounds& left, const LabelBounds& right)
    {
        return left.minimum.x < right.maximum.x
            && left.maximum.x > right.minimum.x
            && left.minimum.y < right.maximum.y
            && left.maximum.y > right.minimum.y;
    }

    ImVec2 chooseClearLabelPosition(
        ImVec2 anchor,
        ImVec2 textSize,
        float anchorClearance,
        ImVec2 canvasMinimum,
        ImVec2 canvasMaximum,
        const std::vector<std::pair<ImVec2, ImVec2>>& routePolylines,
        const std::vector<LabelBounds>& placedLabels)
    {
        constexpr ImVec2 directions[] = {
            {1.0f, 0.0f}, {1.0f, -1.0f}, {0.0f, -1.0f}, {-1.0f, -1.0f},
            {-1.0f, 0.0f}, {-1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
        const float halfDiagonal = std::sqrt(textSize.x * textSize.x + textSize.y * textSize.y) * 0.5f;
        ImVec2 bestPosition = anchor;
        float bestScore = -std::numeric_limits<float>::infinity();

        for (ImVec2 direction : directions) {
            const float directionLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            direction.x /= directionLength;
            direction.y /= directionLength;
            ImVec2 position(
                anchor.x + direction.x * (anchorClearance + halfDiagonal) - textSize.x * 0.5f,
                anchor.y + direction.y * (anchorClearance + halfDiagonal) - textSize.y * 0.5f);
            position.x = std::clamp(position.x, canvasMinimum.x + 3.0f, canvasMaximum.x - textSize.x - 3.0f);
            position.y = std::clamp(position.y, canvasMinimum.y + 3.0f, canvasMaximum.y - textSize.y - 3.0f);

            const LabelBounds candidate{position, ImVec2(position.x + textSize.x, position.y + textSize.y)};
            const ImVec2 samples[] = {
                ImVec2((candidate.minimum.x + candidate.maximum.x) * 0.5f, (candidate.minimum.y + candidate.maximum.y) * 0.5f),
                candidate.minimum,
                candidate.maximum,
                ImVec2(candidate.minimum.x, candidate.maximum.y),
                ImVec2(candidate.maximum.x, candidate.minimum.y)};
            float routeClearance = 100000.0f;
            for (const auto& segment : routePolylines) {
                for (const ImVec2 sample : samples) {
                    routeClearance = std::min(routeClearance,
                        std::sqrt(distanceSquaredToSegment(sample, segment.first, segment.second)));
                }
            }
            const bool overlapsLabel = std::any_of(placedLabels.begin(), placedLabels.end(),
                [&candidate](const LabelBounds& placed) { return labelsOverlap(candidate, placed); });
            const float score = routeClearance - (overlapsLabel ? 100000.0f : 0.0f);
            if (score > bestScore) {
                bestScore = score;
                bestPosition = position;
            }
        }
        return bestPosition;
    }

    void drawReadableLabel(ImDrawList* drawList, ImVec2 position, const char* text)
    {
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        drawList->AddRectFilled(
            ImVec2(position.x - 3.0f, position.y - 2.0f),
            ImVec2(position.x + textSize.x + 3.0f, position.y + textSize.y + 2.0f),
            IM_COL32(246, 248, 250, 235),
            3.0f);
        drawList->AddRect(
            ImVec2(position.x - 3.0f, position.y - 2.0f),
            ImVec2(position.x + textSize.x + 3.0f, position.y + textSize.y + 2.0f),
            IM_COL32(110, 120, 130, 200),
            3.0f);
        drawList->AddText(position, IM_COL32(20, 30, 40, 255), text);
    }

    void drawRotatedRouteLabel(
        ImDrawList* drawList,
        ImVec2 center,
        ImVec2 direction,
        ImU32 color,
        const char* label)
    {
        const ImVec2 textSize = ImGui::CalcTextSize(label);
        const ImVec2 normal(-direction.y, direction.x);
        const float halfWidth = textSize.x * 0.5f + 4.0f;
        const float halfHeight = textSize.y * 0.5f + 3.0f;
        const ImVec2 topLeft(
            center.x - direction.x * halfWidth - normal.x * halfHeight,
            center.y - direction.y * halfWidth - normal.y * halfHeight);
        const ImVec2 topRight(
            center.x + direction.x * halfWidth - normal.x * halfHeight,
            center.y + direction.y * halfWidth - normal.y * halfHeight);
        const ImVec2 bottomRight(
            center.x + direction.x * halfWidth + normal.x * halfHeight,
            center.y + direction.y * halfWidth + normal.y * halfHeight);
        const ImVec2 bottomLeft(
            center.x - direction.x * halfWidth + normal.x * halfHeight,
            center.y - direction.y * halfWidth + normal.y * halfHeight);
        drawList->AddQuadFilled(
            topLeft, topRight, bottomRight, bottomLeft,
            IM_COL32(246, 248, 250, 235));
        drawList->AddQuad(
            topLeft, topRight, bottomRight, bottomLeft,
            IM_COL32(110, 120, 130, 200),
            1.0f);
        const float angle = std::atan2(direction.y, direction.x);
        const ImVec2 textTopLeft(
            center.x - textSize.x * 0.5f,
            center.y - textSize.y * 0.5f);
        const int firstVertex = drawList->VtxBuffer.Size;
        drawList->AddText(textTopLeft, color, label);

        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        for (int index = firstVertex; index < drawList->VtxBuffer.Size; ++index) {
            ImDrawVert& vertex = drawList->VtxBuffer[index];
            const float localX = vertex.pos.x - center.x;
            const float localY = vertex.pos.y - center.y;
            vertex.pos.x = center.x + localX * cosine - localY * sine;
            vertex.pos.y = center.y + localX * sine + localY * cosine;
        }
    }

} // namespace


void render::drawPhysicalGridView(
    core::World& world,
    const RenderingConfig& config,
    render::Tool activeTool,
    bool modalOpen)
{
    applyGridPanelLayout();

    ImGui::Begin(
        "Physical Grid",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize);

    // ========================================================
    // Zoom limits
    // ========================================================

    const float minimumExtentMeters =
        areaToSquareSide(
            config.minimumGridZoomAreaSquareMeters);

    const float maximumExtentMeters =
        std::max(
            minimumExtentMeters,
            areaToSquareSide(
                config.maximumGridZoomAreaSquareMeters));

    const float configuredDefaultExtent =
        std::clamp(
            areaToSquareSide(
                config.defaultGridZoomAreaSquareMeters),
            minimumExtentMeters,
            maximumExtentMeters);

    GridViewState& state =
        getGridViewState();

    // ========================================================
    // Initialize state
    // ========================================================

    if (!state.initialized)
    {
        state.initialized = true;

        state.visibleExtentMeters =
            configuredDefaultExtent;

        state.pan =
            ImVec2(0.0f, 0.0f);

        state.homeExtentMeters =
            configuredDefaultExtent;

        state.homePan =
            ImVec2(0.0f, 0.0f);
    }

    state.visibleExtentMeters =
        std::clamp(
            state.visibleExtentMeters,
            minimumExtentMeters,
            maximumExtentMeters);

    state.homeExtentMeters =
        std::clamp(
            state.homeExtentMeters,
            minimumExtentMeters,
            maximumExtentMeters);

    // ========================================================
    // Controls
    // ========================================================

    ImGui::TextDisabled(
        "Render grid • %zu physical segment geometries",
        world.getPhysicalNetwork()
        .getSegmentGeometries()
        .size());

    ImGui::SameLine();

    // --------------------------------------------------------
    // Reset View
    //
    // ALWAYS returns the physical origin to the center.
    //
    // Current zoom is preserved.
    // --------------------------------------------------------

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::SmallButton("Reset View"))
    {
        state.pan =
            ImVec2(0.0f, 0.0f);
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // --------------------------------------------------------
    // Home View
    //
    // Restores the saved home camera.
    // --------------------------------------------------------

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::SmallButton("Home View"))
    {
        state.visibleExtentMeters =
            state.homeExtentMeters;

        state.pan =
            state.homePan;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // --------------------------------------------------------
    // Set Home View
    //
    // Saves both zoom and pan.
    // --------------------------------------------------------

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::SmallButton("Set Home View"))
    {
        state.homeExtentMeters =
            state.visibleExtentMeters;

        state.homePan =
            state.pan;
    }
    ImGui::PopStyleColor();

    // ========================================================
    // Manual zoom input
    // ========================================================

    float visibleExtentMeters =
        state.visibleExtentMeters;

    ImGui::SetNextItemWidth(110.0f);

    ui::pushTextInputStyle();
    if (ImGui::InputFloat(
        &visibleExtentMeters,
        0.0f,
        0.0f,
        "%.1f m"))
    {
        state.visibleExtentMeters =
            std::clamp(
                visibleExtentMeters,
                minimumExtentMeters,
                maximumExtentMeters);
    }
    ui::popTextInputStyle();

    ImGui::SameLine();

    // ========================================================
    // Zoom presets
    // ========================================================

    if (ImGui::BeginCombo(
        "##GridExtentPresets",
        "Extent Presets"))
    {
        constexpr float presets[] = {
            5.0f,
            10.0f,
            25.0f,
            50.0f,
            100.0f,
            250.0f,
            500.0f
        };

        for (float preset : presets)
        {
            if (preset < minimumExtentMeters ||
                preset > maximumExtentMeters)
            {
                continue;
            }

            char label[32];

            std::snprintf(
                label,
                sizeof(label),
                "%.0f m x %.0f m",
                preset,
                preset);

            if (ImGui::Selectable(
                label,
                std::abs(
                    state.visibleExtentMeters -
                    preset) < 0.01f))
            {
                state.visibleExtentMeters =
                    preset;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::TextDisabled(
        "Viewing %.1f m x %.1f m • allowed: %.1f m² to %.1f m²",
        state.visibleExtentMeters,
        state.visibleExtentMeters,
        config.minimumGridZoomAreaSquareMeters,
        config.maximumGridZoomAreaSquareMeters);

    ImGui::Separator();

    // ========================================================
    // Canvas geometry
    // ========================================================

    const ImVec2 canvasPosition =
        ImGui::GetCursorScreenPos();

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const float limitingCanvasSize =
        std::max(
            1.0f,
            std::min(
                available.x,
                available.y));

    const float pixelsPerMeter =
        limitingCanvasSize /
        state.visibleExtentMeters;

    const float canvasRight =
        canvasPosition.x +
        available.x;

    const float canvasBottom =
        canvasPosition.y +
        available.y;

    // ========================================================
    // Physical origin
    //
    // When:
    //
    //     state.pan == (0,0)
    //
    // physical:
    //
    //     (0,0,0)
    //
    // appears exactly at the center of the canvas.
    // ========================================================

    const ImVec2 origin(
        canvasPosition.x +
        available.x * 0.5f +
        state.pan.x,

        canvasPosition.y +
        available.y * 0.5f +
        state.pan.y);

    // ========================================================
    // Canvas background
    // ========================================================

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    drawList->PushClipRect(
        canvasPosition,
        ImVec2(
            canvasRight,
            canvasBottom),
        true);

    drawList->AddRectFilled(
        canvasPosition,
        ImVec2(
            canvasRight,
            canvasBottom),
        IM_COL32(
            246,
            248,
            250,
            255));

    drawList->PopClipRect();

    // ========================================================
    // Saved routes
    //
    // A route stores abstract node IDs in travel order. Its rendered physical
    // geometry is the straight sequence of the matching physical node positions.
    // ========================================================

    drawList->PushClipRect(
        canvasPosition,
        ImVec2(canvasRight, canvasBottom),
        true);

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    AbstractRouteId hoveredRoute{};
    bool hasHoveredRoute = false;
    float closestRouteDistanceSquared = 0.0f;
    std::vector<std::pair<ImVec2, ImVec2>> routePolylines;
    std::vector<LabelBounds> placedLabels;

    world.forEachRoute([&](AbstractRouteId routeId, network::AbstractRoute& route) {
        const std::vector<AbstractNodeId>& routeNodes = route.getNodes();
        if (routeNodes.size() < 2) {
            return;
        }

        const bool isSelected = world.getActiveRoute().has_value()
            && world.getActiveRoute()->id == routeId.id
            && world.getActiveRoute()->generation == routeId.generation;
        const network::RouteColor& routeColor = route.getColor();
        const ImU32 lineColor = IM_COL32(
            static_cast<int>(std::clamp(routeColor.r, 0.0f, 1.0f) * 255.0f),
            static_cast<int>(std::clamp(routeColor.g, 0.0f, 1.0f) * 255.0f),
            static_cast<int>(std::clamp(routeColor.b, 0.0f, 1.0f) * 255.0f),
            235);
        const float lineWidth = std::clamp(pixelsPerMeter * 0.20f, 2.5f, 6.0f)
            + (isSelected ? 2.0f : 0.0f);
        ImVec2 centroid(0.0f, 0.0f);
        ImVec2 longestStart{};
        ImVec2 longestEnd{};
        float longestLengthSquared = 0.0f;

        for (AbstractNodeId nodeId : routeNodes) {
            const ImVec2 nodePosition = physicalToScreen(
                world.getPhysicalNode(nodeId).getCoordinate(), origin, pixelsPerMeter);
            centroid.x += nodePosition.x;
            centroid.y += nodePosition.y;
        }
        centroid.x /= static_cast<float>(routeNodes.size());
        centroid.y /= static_cast<float>(routeNodes.size());

        for (size_t index = 1; index < routeNodes.size(); ++index) {
            const ImVec2 start = physicalToScreen(
                world.getPhysicalNode(routeNodes[index - 1]).getCoordinate(),
                origin,
                pixelsPerMeter);
            const ImVec2 end = physicalToScreen(
                world.getPhysicalNode(routeNodes[index]).getCoordinate(),
                origin,
                pixelsPerMeter);
            drawList->AddLine(start, end, lineColor, lineWidth);
            routePolylines.emplace_back(start, end);

            const float deltaX = end.x - start.x;
            const float deltaY = end.y - start.y;
            const float lengthSquared = deltaX * deltaX + deltaY * deltaY;
            if (lengthSquared > longestLengthSquared) {
                longestLengthSquared = lengthSquared;
                longestStart = start;
                longestEnd = end;
            }

            const float distanceSquared = distanceSquaredToSegment(mouse, start, end);
            const float hitRadius = std::max(10.0f, lineWidth + 5.0f);
            if (distanceSquared <= hitRadius * hitRadius
                && (!hasHoveredRoute || distanceSquared < closestRouteDistanceSquared)) {
                hoveredRoute = routeId;
                hasHoveredRoute = true;
                closestRouteDistanceSquared = distanceSquared;
            }
        }

        if (!route.getName().empty() && longestLengthSquared >= 900.0f) {
            const float length = std::sqrt(longestLengthSquared);
            ImVec2 direction(
                (longestEnd.x - longestStart.x) / length,
                (longestEnd.y - longestStart.y) / length);
            ImVec2 normal(-direction.y, direction.x);
            const ImVec2 midpoint(
                (longestStart.x + longestEnd.x) * 0.5f,
                (longestStart.y + longestEnd.y) * 0.5f);
            const float insideSide = (centroid.x - midpoint.x) * normal.x
                + (centroid.y - midpoint.y) * normal.y;
            if (insideSide < 0.0f) {
                normal.x = -normal.x;
                normal.y = -normal.y;
            }
            if (direction.x < 0.0f) {
                direction.x = -direction.x;
                direction.y = -direction.y;
            }
            const ImVec2 labelCenter(
                midpoint.x + normal.x * (lineWidth + 8.0f),
                midpoint.y + normal.y * (lineWidth + 8.0f));
            drawRotatedRouteLabel(
                drawList,
                labelCenter,
                direction,
                IM_COL32(20, 30, 40, 230),
                route.getName().c_str());
            const ImVec2 routeLabelSize = ImGui::CalcTextSize(route.getName().c_str());
            const float routeLabelRadius = std::hypot(routeLabelSize.x, routeLabelSize.y) * 0.5f + 5.0f;
            placedLabels.push_back({
                ImVec2(labelCenter.x - routeLabelRadius, labelCenter.y - routeLabelRadius),
                ImVec2(labelCenter.x + routeLabelRadius, labelCenter.y + routeLabelRadius)});
        }
    });

    // Vehicle poses are derived by World from each vehicle's route progress and
    // the route's physical node geometry. Rendering remains presentation-only.
    VehicleId hoveredVehicle{};
    bool hasHoveredVehicle = false;
    float closestVehicleDistanceSquared = 0.0f;
    world.forEachVehicle([&](VehicleId vehicleId, vehicle::Vehicle& vehicle) {
        const std::optional<core::PhysicalVehiclePose> pose = world.getVehiclePose(vehicleId);
        if (!pose.has_value()) {
            return;
        }

        const ImVec2 center = physicalToScreen(pose->coordinate, origin, pixelsPerMeter);
        if (center.x < canvasPosition.x - 40.0f || center.x > canvasRight + 40.0f
            || center.y < canvasPosition.y - 40.0f || center.y > canvasBottom + 40.0f) {
            return;
        }

        const ImVec2 direction(pose->direction.x, -pose->direction.y);
        const ImVec2 normal(-direction.y, direction.x);
        const float halfLength = std::clamp(pixelsPerMeter * 1.1f, 9.0f, 34.0f);
        const float halfWidth = std::clamp(pixelsPerMeter * 0.45f, 5.0f, 14.0f);
        const ImVec2 frontLeft(center.x + direction.x * halfLength + normal.x * halfWidth,
            center.y + direction.y * halfLength + normal.y * halfWidth);
        const ImVec2 frontRight(center.x + direction.x * halfLength - normal.x * halfWidth,
            center.y + direction.y * halfLength - normal.y * halfWidth);
        const ImVec2 rearRight(center.x - direction.x * halfLength - normal.x * halfWidth,
            center.y - direction.y * halfLength - normal.y * halfWidth);
        const ImVec2 rearLeft(center.x - direction.x * halfLength + normal.x * halfWidth,
            center.y - direction.y * halfLength + normal.y * halfWidth);
        drawList->AddQuadFilled(frontLeft, frontRight, rearRight, rearLeft, IM_COL32(32, 61, 92, 255));
        drawList->AddQuad(frontLeft, frontRight, rearRight, rearLeft, IM_COL32(245, 248, 250, 255), 1.5f);
        drawList->AddLine(frontLeft, frontRight, IM_COL32(90, 196, 232, 255), 2.0f);
        char vehicleLabel[128];
        if (ui::shouldShowVehicleSpeeds()) {
            std::snprintf(vehicleLabel, sizeof(vehicleLabel), "%s (%.0f km/h)",
                vehicle.getDisplayName().c_str(), vehicle.getCurrentSpeedKph());
        } else {
            std::snprintf(vehicleLabel, sizeof(vehicleLabel), "%s", vehicle.getDisplayName().c_str());
        }
        const ImVec2 vehicleLabelSize = ImGui::CalcTextSize(vehicleLabel);
        const ImVec2 vehicleLabelPosition = chooseClearLabelPosition(
            center,
            vehicleLabelSize,
            std::max(halfLength, halfWidth) + 7.0f,
            canvasPosition,
            ImVec2(canvasRight, canvasBottom),
            routePolylines,
            placedLabels);
        drawReadableLabel(drawList, vehicleLabelPosition, vehicleLabel);
        placedLabels.push_back({
            ImVec2(vehicleLabelPosition.x - 3.0f, vehicleLabelPosition.y - 2.0f),
            ImVec2(vehicleLabelPosition.x + vehicleLabelSize.x + 3.0f,
                vehicleLabelPosition.y + vehicleLabelSize.y + 2.0f)});

        const float mouseDeltaX = mouse.x - center.x;
        const float mouseDeltaY = mouse.y - center.y;
        const float vehicleDistanceSquared = mouseDeltaX * mouseDeltaX + mouseDeltaY * mouseDeltaY;
        const float vehicleHoverRadius = std::max(halfLength, halfWidth) + 6.0f;
        if (vehicleDistanceSquared <= vehicleHoverRadius * vehicleHoverRadius
            && (!hasHoveredVehicle || vehicleDistanceSquared < closestVehicleDistanceSquared)) {
            hoveredVehicle = vehicleId;
            hasHoveredVehicle = true;
            closestVehicleDistanceSquared = vehicleDistanceSquared;
        }
    });

    drawList->PopClipRect();

    // ========================================================
    // Active node markers
    // ========================================================

    AbstractNodeId hoveredNode{};
    bool hasHoveredNode = false;
    float closestNodeDistanceSquared = 0.0f;
    const float nodeMarkerRadius = std::clamp(pixelsPerMeter * 0.22f, 4.0f, 9.0f);

    drawList->PushClipRect(
        canvasPosition,
        ImVec2(canvasRight, canvasBottom),
        true);

    world.forEachNode([&](AbstractNodeId id, network::AbstractNode& node) {
        const network::PhysicalNode& physicalNode = world.getPhysicalNode(id);
        const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();
        const ImVec2 nodeScreen = physicalToScreen(coordinate, origin, pixelsPerMeter);

        if (nodeScreen.x < canvasPosition.x || nodeScreen.x > canvasRight
            || nodeScreen.y < canvasPosition.y || nodeScreen.y > canvasBottom) {
            return;
        }

        const bool isDragging = state.draggingNode.has_value()
            && state.draggingNode->id == id.id
            && state.draggingNode->generation == id.generation;
        const bool isRouteSelected = ui::isRouteNodeSelected(id);
        const bool isSelected = world.getActiveNode().has_value()
            && world.getActiveNode()->id == id.id
            && world.getActiveNode()->generation == id.generation;
        drawList->AddCircleFilled(
            nodeScreen,
            nodeMarkerRadius,
            isRouteSelected
                ? IM_COL32(170, 48, 56, 255)
                : isSelected
                ? IM_COL32(190, 55, 63, isDragging ? 80 : 255)
                : IM_COL32(112, 116, 120, isDragging ? 80 : 230));
        drawList->AddCircle(
            nodeScreen,
            nodeMarkerRadius + 1.0f,
            IM_COL32(20, 35, 50, 220));

        std::string label = node.getName() + " (" + network::nodeTypeName(physicalNode.getNodeType()) + ")";
        const ImVec2 nodeLabelSize = ImGui::CalcTextSize(label.c_str());
        const ImVec2 nodeLabelPosition = chooseClearLabelPosition(
            nodeScreen,
            nodeLabelSize,
            nodeMarkerRadius + 7.0f,
            canvasPosition,
            ImVec2(canvasRight, canvasBottom),
            routePolylines,
            placedLabels);
        drawReadableLabel(drawList, nodeLabelPosition, label.c_str());
        placedLabels.push_back({
            ImVec2(nodeLabelPosition.x - 3.0f, nodeLabelPosition.y - 2.0f),
            ImVec2(nodeLabelPosition.x + nodeLabelSize.x + 3.0f,
                nodeLabelPosition.y + nodeLabelSize.y + 2.0f)});

        const float distanceX = mouse.x - nodeScreen.x;
        const float distanceY = mouse.y - nodeScreen.y;
        const float distanceSquared = distanceX * distanceX + distanceY * distanceY;
        const float hoverRadius = std::max(10.0f, nodeMarkerRadius + 3.0f);
        if (distanceSquared <= hoverRadius * hoverRadius
            && (!hasHoveredNode || distanceSquared < closestNodeDistanceSquared)) {
            hoveredNode = id;
            hasHoveredNode = true;
            closestNodeDistanceSquared = distanceSquared;
        }
    });

    drawList->PopClipRect();

    // ========================================================
    // Visible physical range
    //
    // Only lines that can appear on screen are generated.
    // ========================================================

    const float visibleHalfWidthMeters =
        available.x * 0.5f /
        pixelsPerMeter;

    const float visibleHalfHeightMeters =
        available.y * 0.5f /
        pixelsPerMeter;

    const double panWorldX =
        static_cast<double>(
            state.pan.x) /
        pixelsPerMeter;

    const double panWorldY =
        static_cast<double>(
            state.pan.y) /
        pixelsPerMeter;

    const int minX =
        static_cast<int>(
            std::floor(
                -visibleHalfWidthMeters -
                panWorldX));

    const int maxX =
        static_cast<int>(
            std::ceil(
                visibleHalfWidthMeters -
                panWorldX));

    const int minY =
        static_cast<int>(
            std::floor(
                -visibleHalfHeightMeters +
                panWorldY));

    const int maxY =
        static_cast<int>(
            std::ceil(
                visibleHalfHeightMeters +
                panWorldY));

    // ========================================================
    // Grid LOD
    // ========================================================

    const float gridStepMeters =
        chooseGridStep(
            pixelsPerMeter);

    const float gridStepPixels =
        gridStepMeters *
        pixelsPerMeter;

    const double majorStepMeters =
        static_cast<double>(
            gridStepMeters) *
        5.0;

    const ImU32 minorGridColour =
        IM_COL32(
            142,
            157,
            172,
            34);

    const ImU32 majorGridColour =
        IM_COL32(
            112,
            128,
            145,
            85);

    const ImU32 axisColour =
        IM_COL32(
            80,
            95,
            110,
            150);

    // ========================================================
    // Grid rendering
    // ========================================================

    const double firstGridX =
        std::ceil(
            static_cast<double>(minX) /
            gridStepMeters) *
        gridStepMeters;

    const double firstGridY =
        std::ceil(
            static_cast<double>(minY) /
            gridStepMeters) *
        gridStepMeters;

    drawList->PushClipRect(
        canvasPosition,
        ImVec2(
            canvasRight,
            canvasBottom),
        true);

    // --------------------------------------------------------
    // Vertical lines
    //
    // Constant physical X.
    // --------------------------------------------------------

    for (double x = firstGridX;
        x <=
        static_cast<double>(maxX) +
        gridStepMeters * 0.5;
        x += gridStepMeters)
    {
        const float screenX =
            origin.x +
            static_cast<float>(
                x * pixelsPerMeter);

        const bool isAxis =
            std::abs(x) <
            gridStepMeters * 0.001;

        const bool isMajor =
            std::abs(
                std::fmod(
                    std::abs(x),
                    majorStepMeters)) <
            gridStepMeters * 0.001;

        drawList->AddLine(
            ImVec2(
                screenX,
                canvasPosition.y),

            ImVec2(
                screenX,
                canvasBottom),

            isAxis
            ? axisColour
            : (isMajor
                ? majorGridColour
                : minorGridColour));
    }

    // --------------------------------------------------------
    // Horizontal lines
    //
    // Constant physical Y.
    //
    // Physical +Y points upward, so screen Y is inverted.
    // --------------------------------------------------------

    for (double y = firstGridY;
        y <=
        static_cast<double>(maxY) +
        gridStepMeters * 0.5;
        y += gridStepMeters)
    {
        const float screenY =
            origin.y -
            static_cast<float>(
                y * pixelsPerMeter);

        const bool isAxis =
            std::abs(y) <
            gridStepMeters * 0.001;

        const bool isMajor =
            std::abs(
                std::fmod(
                    std::abs(y),
                    majorStepMeters)) <
            gridStepMeters * 0.001;

        drawList->AddLine(
            ImVec2(
                canvasPosition.x,
                screenY),

            ImVec2(
                canvasRight,
                screenY),

            isAxis
            ? axisColour
            : (isMajor
                ? majorGridColour
                : minorGridColour));
    }

    // --------------------------------------------------------
    // Explicit X/Y axes
    //
    // These are already included above, but keeping the
    // coordinate axes visually distinct is intentional.
    // --------------------------------------------------------

    // Live physical scale: the bar always represents the labelled number of metres
    // at the current zoom, independent of the panel's pixel dimensions.
    const float scaleBarMeters = chooseGridStep(pixelsPerMeter);
    const float scaleBarPixels = scaleBarMeters * pixelsPerMeter;
    const ImVec2 scaleStart(canvasPosition.x + 16.0f, canvasBottom - 18.0f);
    const ImVec2 scaleEnd(scaleStart.x + scaleBarPixels, scaleStart.y);
    char scaleLabel[32];
    formatDistanceLabel(scaleBarMeters, scaleLabel, sizeof(scaleLabel));
    const ImVec2 scaleTextSize = ImGui::CalcTextSize(scaleLabel);
    drawList->AddRectFilled(
        ImVec2(scaleStart.x - 7.0f, scaleStart.y - scaleTextSize.y - 12.0f),
        ImVec2(scaleEnd.x + 7.0f, scaleStart.y + 7.0f),
        IM_COL32(246, 248, 250, 225),
        3.0f);
    drawList->AddLine(scaleStart, scaleEnd, IM_COL32(20, 30, 40, 255), 2.0f);
    drawList->AddLine(
        ImVec2(scaleStart.x, scaleStart.y - 5.0f),
        ImVec2(scaleStart.x, scaleStart.y + 5.0f),
        IM_COL32(20, 30, 40, 255), 2.0f);
    drawList->AddLine(
        ImVec2(scaleEnd.x, scaleEnd.y - 5.0f),
        ImVec2(scaleEnd.x, scaleEnd.y + 5.0f),
        IM_COL32(20, 30, 40, 255), 2.0f);
    drawList->AddText(
        ImVec2(scaleStart.x, scaleStart.y - scaleTextSize.y - 6.0f),
        IM_COL32(20, 30, 40, 255),
        scaleLabel);

    drawList->PopClipRect();

    // ========================================================
    // Canvas interaction
    // ========================================================

    ImGui::InvisibleButton(
        "PhysicalGridCanvas",
        available);

    const bool canvasHovered =
        ImGui::IsMouseHoveringRect(
            canvasPosition,
            ImVec2(
                canvasRight,
                canvasBottom),
            true);

    if (canvasHovered && !modalOpen)
    {
        ImGui::SetItemKeyOwner(
            ImGuiKey_MouseWheelY);

        const ImVec2 mouse = ImGui::GetIO().MousePos;

        // ====================================================
        // Zoom
        //
        // Preserve the physical coordinate underneath the
        // mouse cursor.
        // ====================================================

        if (ImGui::GetIO().MouseWheel != 0.0f)
        {
            const network::PhysicalCoordinate mousePhysical =
                screenToPhysical(
                    mouse,
                    origin,
                    pixelsPerMeter);

            const float nextExtent =
                std::clamp(
                    state.visibleExtentMeters /
                    std::pow(
                        1.15f,
                        ImGui::GetIO().MouseWheel),

                    minimumExtentMeters,
                    maximumExtentMeters);

            const float nextPixelsPerMeter =
                limitingCanvasSize /
                nextExtent;

            // Position the new origin so that the same
            // physical coordinate remains underneath
            // the mouse.
            const ImVec2 nextOrigin(
                mouse.x -
                static_cast<float>(
                    mousePhysical.x *
                    nextPixelsPerMeter),

                mouse.y +
                static_cast<float>(
                    mousePhysical.y *
                    nextPixelsPerMeter));

            state.pan = ImVec2(
                nextOrigin.x -
                (canvasPosition.x +
                    available.x * 0.5f),

                nextOrigin.y -
                (canvasPosition.y +
                    available.y * 0.5f));

            state.visibleExtentMeters =
                nextExtent;
        }

        // ====================================================
        // Middle mouse pan
        // ====================================================

        if (ImGui::IsMouseDragging(
            ImGuiMouseButton_Middle))
        {
            state.pan.x +=
                ImGui::GetIO().MouseDelta.x;

            state.pan.y +=
                ImGui::GetIO().MouseDelta.y;
        }

        // ====================================================
        // Recalculate origin after zoom/pan.
        // ====================================================

        const ImVec2 currentOrigin(
            canvasPosition.x +
            available.x * 0.5f +
            state.pan.x,

            canvasPosition.y +
            available.y * 0.5f +
            state.pan.y);

        // ====================================================
        // Mouse → physical coordinate
        // ====================================================

        const network::PhysicalCoordinate mousePhysical =
            screenToPhysical(
                mouse,
                currentOrigin,
                pixelsPerMeter);

        // ====================================================
        // Snap to physical lattice
        //
        // Current plane:
        //
        //     Z = 0
        //
        // Lattice spacing:
        //
        //     1 m
        // ====================================================



        const network::PhysicalCoordinate snapped{
            zeroRound(mousePhysical.x),
            zeroRound(mousePhysical.y),
            zeroRound(mousePhysical.z)
        };
        const bool isNodePlacementTool = activeTool == render::Tool::Node;

        const ImVec2 snappedScreen =
            physicalToScreen(
                snapped,
                currentOrigin,
                pixelsPerMeter);

        // ====================================================
        // Draw snapped point
        // ====================================================

        const bool snappedVisible =
            snappedScreen.x >=
            canvasPosition.x &&

            snappedScreen.x <=
            canvasRight &&

            snappedScreen.y >=
            canvasPosition.y &&

            snappedScreen.y <=
            canvasBottom;

        if (isNodePlacementTool && snappedVisible)
        {
            drawList->PushClipRect(
                canvasPosition,
                ImVec2(
                    canvasRight,
                    canvasBottom),
                true);

            drawList->AddCircleFilled(
                snappedScreen,

                std::clamp(
                    pixelsPerMeter * 0.18f,
                    1.5f,
                    7.0f),

                IM_COL32(
                    0,
                    0,
                    0,
                    105));

            drawList->PopClipRect();

            ImGui::SetTooltip(
                "Snapped point "
                "(%.0f, %.0f, %.0f) m\n"
                "Scroll to zoom • "
                "middle-drag to pan",

                zeroRound(snapped.x),
                zeroRound(snapped.y),
                zeroRound(snapped.z));
        }

            if (hasHoveredNode)
            {
                const network::AbstractNode& node = world.getNode(hoveredNode);
                const network::PhysicalNode& physicalNode = world.getPhysicalNode(hoveredNode);
                const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();
                ImGui::SetTooltip(
                "%s\nType: %s\nCoordinate: (%.0f, %.0f, %.0f) m",
                node.getName().c_str(),
                network::nodeTypeName(physicalNode.getNodeType()),
                zeroRound(coordinate.x),
                zeroRound(coordinate.y),
                zeroRound(coordinate.z));
            }
            else if (hasHoveredVehicle)
            {
                bool vehicleFound = false;
                world.forEachVehicle([&](VehicleId vehicleId, vehicle::Vehicle& vehicle) {
                    if (vehicleId.id == hoveredVehicle.id
                        && vehicleId.generation == hoveredVehicle.generation) {
                        ImGui::SetTooltip("%s\nCurrent speed: %.1f km/h",
                            vehicle.getDisplayName().c_str(), vehicle.getCurrentSpeedKph());
                        vehicleFound = true;
                    }
                });
                (void)vehicleFound;
            }

        if (state.draggingNode.has_value() && state.dragMoved)
        {
            const network::PhysicalCoordinate& originalCoordinate =
                world.getPhysicalNode(*state.draggingNode).getCoordinate();
            const network::PhysicalCoordinate dragCoordinate{
                zeroRound(mousePhysical.x),
                zeroRound(mousePhysical.y),
                zeroRound(originalCoordinate.z)};
            const ImVec2 dragScreen = physicalToScreen(
                dragCoordinate,
                currentOrigin,
                pixelsPerMeter);
            drawList->PushClipRect(
                canvasPosition,
                ImVec2(canvasRight, canvasBottom),
                true);
            drawList->AddCircleFilled(
                dragScreen,
                std::clamp(pixelsPerMeter * 0.22f, 4.0f, 9.0f),
                IM_COL32(165, 70, 68, 230));
            drawList->PopClipRect();
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)
            && activeTool == render::Tool::Pointer
            && hasHoveredNode)
        {
            world.setActiveNodeAt(world.getPhysicalNode(hoveredNode).getCoordinate());
            ui::requestNodeEdit(world, hoveredNode);
        }
        else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
            && activeTool == render::Tool::Pointer
            && hasHoveredNode)
        {
            world.setActiveNodeAt(world.getPhysicalNode(hoveredNode).getCoordinate());
            ui::requestNodeEdit(world, hoveredNode);
        }
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)
            && activeTool == render::Tool::Pointer
            && !hasHoveredNode
            && hasHoveredRoute)
        {
            world.setActiveRoute(hoveredRoute);
            ui::requestRouteEdit(world, hoveredRoute);
        }
        else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
            && activeTool == render::Tool::Pointer
            && !hasHoveredNode
            && hasHoveredRoute)
        {
            world.setActiveRoute(hoveredRoute);
            ui::requestRouteEdit(world, hoveredRoute);
        }
        else if (activeTool == render::Tool::Pointer
            && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && hasHoveredNode)
        {
            state.draggingNode = hoveredNode;
            state.dragStartScreen = mouse;
            state.dragMoved = false;
            world.setActiveNodeAt(world.getPhysicalNode(hoveredNode).getCoordinate());
        }
        else if (activeTool == render::Tool::Pointer
            && state.draggingNode.has_value()
            && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            const float deltaX = mouse.x - state.dragStartScreen.x;
            const float deltaY = mouse.y - state.dragStartScreen.y;
            state.dragMoved = state.dragMoved || deltaX * deltaX + deltaY * deltaY > 16.0f;
        }
        else if (activeTool == render::Tool::Pointer
            && state.draggingNode.has_value()
            && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (state.dragMoved) {
                const network::PhysicalCoordinate dropCoordinate{
                    zeroRound(mousePhysical.x),
                    zeroRound(mousePhysical.y),
                    zeroRound(world.getPhysicalNode(*state.draggingNode).getCoordinate().z)};
                ui::requestNodeMove(world, *state.draggingNode, dropCoordinate);
            }
            state.draggingNode.reset();
            state.dragMoved = false;
        }
        else if (activeTool == render::Tool::Pointer
            && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && hasHoveredRoute)
        {
            world.setActiveRoute(hoveredRoute);
        }
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (activeTool == render::Tool::Node) {
                if (!world.hasNodeAt(snapped)) {
                    ui::requestNodeCreation(snapped);
                }
            } else if (activeTool == render::Tool::Route && hasHoveredNode) {
                ui::toggleRouteNode(hoveredNode);
            } else {
                world.setActiveNodeAt(mousePhysical);
            }
        }
    }

    ImGui::End();
}
