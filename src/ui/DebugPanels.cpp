#include "ui/DebugPanels.hpp"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <cstring>

#include "vehicle/Vehicle.hpp"
#include "core/World.hpp"

namespace ui
{
    namespace
    {
        void setInitialWindowPosition(ImVec2 position, ImVec2 size) {
            ImGui::SetNextWindowPos(position, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
        }

        struct PhysicalGridViewState
        {
            float zoom = 1.0f;
            ImVec2 pan{0.0f, 0.0f};
        };

        PhysicalGridViewState& getPhysicalGridViewState() {
            static PhysicalGridViewState state;
            return state;
        }
    }

    LogBuffer& LogBuffer::instance() {
        static LogBuffer buffer;
        return buffer;
    }

    void LogBuffer::add(const std::string& message) {
        this->lines.push_back(message);
        if (this->lines.size() > MAX_LINES) {
            this->lines.erase(this->lines.begin());
        }
    }

    void LogBuffer::clear() {
        this->lines.clear();
    }

    const std::vector<std::string>& LogBuffer::getLines() const {
        return this->lines;
    }

    VehicleCreationDraft& getVehicleCreationDraft() {
        static VehicleCreationDraft draft;
        return draft;
    }

    void loadVehicleCreationPreset(
        const char* displayName,
        float maxSpeedKph,
        float accelerationKphPerSecond,
        float decelerationKphPerSecond,
        int passengerCapacity) {
        VehicleCreationDraft& draft = getVehicleCreationDraft();
        std::strncpy(draft.displayName, displayName, sizeof(draft.displayName) - 1);
        draft.displayName[sizeof(draft.displayName) - 1] = '\0';
        draft.maxSpeedKph = maxSpeedKph;
        draft.accelerationKphPerSecond = accelerationKphPerSecond;
        draft.decelerationKphPerSecond = decelerationKphPerSecond;
        draft.passengerCapacity = passengerCapacity;
    }

    namespace
    {
        bool showVehicleCreationModal = false;
        bool showNodeCreationModal = false;

        void openVehicleCreationModal() { showVehicleCreationModal = true; }
        void openNodeCreationModal() { showNodeCreationModal = true; }

        bool createVehicleFromDraft(core::World& world, VehicleCreationDraft& draft) {
            if (draft.displayName[0] == '\0') {
                LogBuffer::instance().add("Vehicle creation failed: display name is required.");
                return false;
            }

            if (draft.maxSpeedKph <= 0.0f
                || draft.accelerationKphPerSecond <= 0.0f
                || draft.decelerationKphPerSecond <= 0.0f
                || draft.passengerCapacity <= 0) {
                LogBuffer::instance().add("Vehicle creation failed: all numeric properties must be positive.");
                return false;
            }

            world.addVehicle(vehicle::Vehicle(
                draft.displayName,
                draft.maxSpeedKph,
                draft.accelerationKphPerSecond,
                draft.decelerationKphPerSecond,
                draft.passengerCapacity));

            LogBuffer::instance().add(
                std::string("Created vehicle \"") + draft.displayName + "\".");
            return true;
        }

        void drawVehicleCreationModal(core::World& world) {
            if (showVehicleCreationModal) {
                ImGui::OpenPopup("Create Vehicle");
            }

            ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
            if (ImGui::BeginPopupModal("Create Vehicle", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                VehicleCreationDraft& draft = getVehicleCreationDraft();

                ImGui::InputText("Display Name", draft.displayName, sizeof(draft.displayName));
                ImGui::InputFloat("Max Speed (km/h)", &draft.maxSpeedKph, 1.0f, 10.0f, "%.1f");
                ImGui::InputFloat("Acceleration (km/h/s)", &draft.accelerationKphPerSecond, 1.0f, 5.0f, "%.1f");
                ImGui::InputFloat("Deceleration (km/h/s)", &draft.decelerationKphPerSecond, 1.0f, 5.0f, "%.1f");
                ImGui::InputInt("Passenger Capacity", &draft.passengerCapacity, 1, 10);

                ImGui::Separator();
                ImGui::TextDisabled("Presets");

                if (ImGui::Button("Car")) {
                    loadVehicleCreationPreset("Car", 100.0f, 10.0f, 10.0f, 4);
                }
                ImGui::SameLine();
                if (ImGui::Button("Truck")) {
                    loadVehicleCreationPreset("Truck", 90.0f, 8.0f, 8.0f, 2);
                }
                ImGui::SameLine();
                if (ImGui::Button("Light Rail")) {
                    loadVehicleCreationPreset("Light Rail", 60.0f, 12.0f, 12.0f, 150);
                }
                ImGui::SameLine();
                if (ImGui::Button("Subway")) {
                    loadVehicleCreationPreset("Subway", 70.0f, 20.0f, 20.0f, 200);
                }
                ImGui::SameLine();
                if (ImGui::Button("Regional Train")) {
                    loadVehicleCreationPreset("Regional Train", 120.0f, 20.0f, 20.0f, 300);
                }
                ImGui::SameLine();
                if (ImGui::Button("HSR")) {
                    loadVehicleCreationPreset("HSR", 300.0f, 15.0f, 15.0f, 500);
                }

                ImGui::Separator();

                if (ImGui::Button("Confirm", ImVec2(120, 0))) {
                    if (createVehicleFromDraft(world, draft)) {
                        showVehicleCreationModal = false;
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    showVehicleCreationModal = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }
    }

    void drawNodeCreationModal(core::World& world) {
        if (showNodeCreationModal) {
            ImGui::OpenPopup("Create Node");
        }

        ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Create Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

            // ImGui::InputText("Display Name", draft.displayName, sizeof(draft.displayName));

            if (ImGui::Button("Rail Station")) {
                loadVehicleCreationPreset("Car", 100.0f, 10.0f, 10.0f, 4);
            }
            ImGui::SameLine();
            if (ImGui::Button("Bus Stop")) {
                loadVehicleCreationPreset("Truck", 90.0f, 8.0f, 8.0f, 2);
            }

            ImGui::Separator();

            if (ImGui::Button("Confirm", ImVec2(120, 0))) {
                if (true) {
                    showNodeCreationModal = false;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                showNodeCreationModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void drawSimulationControls(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(10, 40), ImVec2(300, 190));
        ImGui::Begin("Simulation Controls");

        const bool running = simulation.isRunning();
        if (running) {
            if (ImGui::Button("Pause")) {
                simulation.setRunning(false);
                LogBuffer::instance().add("Simulation paused.");
            }
        }
        else {
            if (ImGui::Button("Play")) {
                simulation.setRunning(true);
                LogBuffer::instance().add("Simulation started.");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Step Once")) {
            simulation.requestStepOnce();
            LogBuffer::instance().add("Single simulation step requested.");
        }

        ImGui::Separator();

        ImGui::Text("Speed Multiplier");
        float speed = static_cast<float>(simulation.getSpeedMultiplier());
        if (ImGui::SliderFloat("##SpeedMultiplier", &speed, 0.25f, 4.0f, "%.2fx")) {
            simulation.setSpeedMultiplier(speed);
        }

        ImGui::SameLine();
        if (ImGui::Button("0.25x")) { simulation.setSpeedMultiplier(0.25); }
        ImGui::SameLine();
        if (ImGui::Button("1x")) { simulation.setSpeedMultiplier(1.0); }
        ImGui::SameLine();
        if (ImGui::Button("2x")) { simulation.setSpeedMultiplier(2.0); }
        ImGui::SameLine();
        if (ImGui::Button("4x")) { simulation.setSpeedMultiplier(4.0); }

        ImGui::Separator();

        const core::World& world = simulation.getWorld();
        ImGui::Text("Simulation Time: %.2f s", simulation.getSimTime());
        ImGui::Text("Tick Count: %llu", static_cast<unsigned long long>(world.getTickCount()));
        ImGui::Text(
            "Status: %s",
            running ? "Running" : (world.getTickCount() > 0 ? "Paused" : "Stopped"));

        ImGui::End();
    }

    void drawVehicleInspector(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(960, 40), ImVec2(310, 310));
        ImGui::Begin("Vehicle Inspector");

        core::World& world = simulation.getWorld();

        if (ImGui::Button("Create Vehicle")) {
            openVehicleCreationModal();
        }

        drawVehicleCreationModal(world);

        ImGui::Separator();

        if (ImGui::BeginTable("Vehicles", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ScrollX)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Max Speed (km/h)");
            ImGui::TableSetupColumn("Accel (km/h/s)");
            ImGui::TableSetupColumn("Decel (km/h/s)");
            ImGui::TableSetupColumn("Capacity");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            world.forEachVehicle([&](VehicleId id, vehicle::Vehicle& vehicle) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(vehicle.getDisplayName().c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d:%d", id.id, id.generation);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f", vehicle.getMaxOperatingSpeedKph());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f", vehicle.getAccelerationKphPerSecond());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.1f", vehicle.getDecelerationKphPerSecond());

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", vehicle.getPassengerCapacity());

                ImGui::TableSetColumnIndex(6);
                const size_t vehicleIndex = static_cast<size_t>(id.id);
                std::ostringstream removeLabel;
                removeLabel << "Remove##" << id.id << "_" << id.generation;
                if (ImGui::SmallButton(removeLabel.str().c_str())) {
                    world.removeVehicle(vehicleIndex);
                    LogBuffer::instance().add(
                        "Removed vehicle \"" + vehicle.getDisplayName() + "\".");
                }
                });

            ImGui::EndTable();
        }

        ImGui::Text("Total vehicles: %zu", world.getVehicleCount());

        ImGui::End();
    }

    void drawPerformancePanel(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(10, 240), ImVec2(300, 275));
        ImGui::Begin("Performance");

        const double renderLatencyMs = simulation.getLastRenderLatencyMs();
        const double simulationLatencyMs = simulation.getLastSimulationLatencyMs();
        const double overallLatencyMs = simulation.getLastOverallLatencyMs();
        const double targetMs = simulation.getTimestep() * 1000.0;
        const double warningThresholdMs = simulation.getConfig().clockConfig.warningThreshold;
        const bool overallBehind = overallLatencyMs > targetMs + warningThresholdMs;
        const float instantaneousFps = renderLatencyMs > 0.0 ? static_cast<float>(1000.0 / renderLatencyMs) : 0.0f;

        ImGui::SeparatorText("Overall Latency");
        ImGui::TextDisabled("Active work only; excludes VSync and pacing delays");
        ImGui::Text("Overall Latency: %.3f ms", overallLatencyMs);
        ImGui::Text("Render Latency: %.3f ms", renderLatencyMs);
        ImGui::Text("Simulation Latency: %.3f ms", simulationLatencyMs);
        ImGui::Text("Status: %s", overallBehind ? "Behind" : "OK");
        ImGui::TextDisabled("Behind when active work > target (%.3f ms) + %.1f ms",
            targetMs, warningThresholdMs);
        ImGui::Text("Target Timestep: %.3f ms (%d Hz)",
            targetMs, simulation.getConfig().clockConfig.targetSimulationFps);

        ImGui::SeparatorText("Render Latency");
        ImGui::TextDisabled("Draw work only; VSync wait excluded");
        ImGui::Text("Render Latency: %.3f ms", renderLatencyMs);
        ImGui::Text("Instantaneous FPS (Max FPS Potential): %.1f FPS", instantaneousFps);

        ImGui::SeparatorText("Simulation Latency");
        ImGui::TextDisabled("Time spent inside world.tick() on the last frame");
        ImGui::Text("Simulation Latency: %.3f ms", simulationLatencyMs);

        ImGui::Separator();

        bool debugClock = simulation.getConfig().DEBUG_CLOCK;
        if (ImGui::Checkbox("DEBUG_CLOCK console logging", &debugClock)) {
            simulation.getConfig().DEBUG_CLOCK = debugClock;
            LogBuffer::instance().add(
                std::string("DEBUG_CLOCK ") + (debugClock ? "enabled" : "disabled") + ".");
        }
        ImGui::TextDisabled("Prints overall latency warnings to terminal");

        ImGui::End();
    }

    void drawWorldSummary(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(960, 360), ImVec2(310, 190));
        ImGui::Begin("World State");

        const core::World& world = simulation.getWorld();

        ImGui::Text("Vehicles: %zu", world.getVehicleCount());
        ImGui::Text("Abstract Nodes: %zu", world.getAbstractNodeCount());
        ImGui::Text("Abstract Segments: %zu", world.getAbstractSegmentCount());
        ImGui::Text("Abstract Routes: %zu", world.getAbstractRouteCount());
        ImGui::Separator();
        ImGui::Text("Simulation Running: %s", simulation.isRunning() ? "Yes" : "No");
        ImGui::Text("Speed Multiplier: %.2fx", simulation.getSpeedMultiplier());
        ImGui::Text("Tick Count: %llu", static_cast<unsigned long long>(world.getTickCount()));
        ImGui::Text("Simulation Time: %.2f s", simulation.getSimTime());

        ImGui::End();
    }

    void drawPhysicalNetworkView(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(320, 40), ImVec2(630, 670));
        ImGui::Begin("Physical Grid");

        const network::PhysicalNetwork& physicalNetwork =
            simulation.getWorld().getPhysicalNetwork();
        const int gridWidth = physicalNetwork.getGridWidth();
        const int gridHeight = physicalNetwork.getGridHeight();

        ImGui::TextDisabled(
            "PhysicalNetwork • %d x %d cells • simulation and editor geometry",
            gridWidth, gridHeight);
        ImGui::SameLine();

        PhysicalGridViewState& viewState = getPhysicalGridViewState();
        if (ImGui::SmallButton("Reset View")) {
            viewState.zoom = 1.0f;
            viewState.pan = ImVec2(0.0f, 0.0f);
        }
        ImGui::Separator();

        const ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float padding = 18.0f;
        const float baseCellSize = std::max(
            4.0f,
            std::floor(std::min(
                (available.x - padding * 2.0f) / static_cast<float>(gridWidth),
                (available.y - padding * 2.0f) / static_cast<float>(gridHeight))));
        const float cellSize = baseCellSize * viewState.zoom;
        const ImVec2 gridSize(
            cellSize * static_cast<float>(gridWidth),
            cellSize * static_cast<float>(gridHeight));
        const ImVec2 gridOrigin(
            canvasPosition.x + (available.x - gridSize.x) * 0.5f + viewState.pan.x,
            canvasPosition.y + (available.y - gridSize.y) * 0.5f + viewState.pan.y);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(
            canvasPosition,
            ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y), true);
        drawList->AddRectFilled(
            canvasPosition,
            ImVec2(canvasPosition.x + available.x, canvasPosition.y + available.y),
            IM_COL32(246, 248, 250, 255));
        drawList->AddRectFilled(
            gridOrigin,
            ImVec2(gridOrigin.x + gridSize.x, gridOrigin.y + gridSize.y),
            IM_COL32(255, 255, 255, 255));

        const ImU32 minorGridColour = IM_COL32(218, 224, 231, 255);
        const ImU32 majorGridColour = IM_COL32(181, 193, 204, 255);
        for (int x = 0; x <= gridWidth; ++x) {
            const float position = gridOrigin.x + cellSize * static_cast<float>(x);
            drawList->AddLine(ImVec2(position, gridOrigin.y),
                ImVec2(position, gridOrigin.y + gridSize.y),
                x % 5 == 0 ? majorGridColour : minorGridColour);
        }
        for (int y = 0; y <= gridHeight; ++y) {
            const float position = gridOrigin.y + cellSize * static_cast<float>(y);
            drawList->AddLine(ImVec2(gridOrigin.x, position),
                ImVec2(gridOrigin.x + gridSize.x, position),
                y % 5 == 0 ? majorGridColour : minorGridColour);
        }
        drawList->AddRect(gridOrigin,
            ImVec2(gridOrigin.x + gridSize.x, gridOrigin.y + gridSize.y),
            IM_COL32(116, 132, 148, 255));

        char zoomLabel[32];
        std::snprintf(zoomLabel, sizeof(zoomLabel), "Zoom %.0f%%", viewState.zoom * 100.0f);
        drawList->AddText(
            ImVec2(canvasPosition.x + 8.0f, canvasPosition.y + 8.0f),
            IM_COL32(74, 89, 104, 255), zoomLabel);
        drawList->PopClipRect();

        ImGui::InvisibleButton("PhysicalGridCanvas", available);
        if (ImGui::IsItemHovered()) {
            const ImVec2 mouse = ImGui::GetIO().MousePos;

            if (ImGui::GetIO().MouseWheel != 0.0f) {
                const float oldCellSize = cellSize;
                const float nextZoom = std::clamp(
                    viewState.zoom * std::pow(1.15f, ImGui::GetIO().MouseWheel),
                    0.25f, 8.0f);
                const float nextCellSize = baseCellSize * nextZoom;
                const ImVec2 nextGridSize(
                    nextCellSize * static_cast<float>(gridWidth),
                    nextCellSize * static_cast<float>(gridHeight));
                const ImVec2 nextCenteredOrigin(
                    canvasPosition.x + (available.x - nextGridSize.x) * 0.5f,
                    canvasPosition.y + (available.y - nextGridSize.y) * 0.5f);
                const ImVec2 mouseGridPosition(
                    (mouse.x - gridOrigin.x) / oldCellSize,
                    (mouse.y - gridOrigin.y) / oldCellSize);
                viewState.pan = ImVec2(
                    mouse.x - nextCenteredOrigin.x - mouseGridPosition.x * nextCellSize,
                    mouse.y - nextCenteredOrigin.y - mouseGridPosition.y * nextCellSize);
                viewState.zoom = nextZoom;
            }

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                const ImVec2 delta = ImGui::GetIO().MouseDelta;
                viewState.pan.x += delta.x;
                viewState.pan.y += delta.y;
            }

            const int cellX = static_cast<int>((mouse.x - gridOrigin.x) / cellSize);
            const int cellY = static_cast<int>((mouse.y - gridOrigin.y) / cellSize);
            if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight) {
                ImGui::SetTooltip(
                    "Grid cell (%d, %d)\nScroll to zoom • middle-drag to pan\n"
                    "Track and obstruction tools will edit this layer.", cellX, cellY);
            }
        }

        ImGui::End();
    }


    void drawNetworkInspector(Simulation& simulation) {
        setInitialWindowPosition(ImVec2(960, 560), ImVec2(310, 150));
        ImGui::Begin("Abstract Network Inspector");

        core::World& world = simulation.getWorld();

        if (ImGui::Button("Create Abstract Node")) {
            openNodeCreationModal();
        }

        drawNodeCreationModal(world);

        ImGui::Separator();

        // if (ImGui::BeginTable("Nodes", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        //     ImGui::TableSetupColumn("Name");
        //     ImGui::TableSetupColumn("ID");
        //     ImGui::TableSetupColumn("Max Speed (km/h)");
        //     ImGui::TableSetupColumn("Accel (km/h/s)");
        //     ImGui::TableSetupColumn("Decel (km/h/s)");
        //     ImGui::TableSetupColumn("Capacity");
        //     ImGui::TableSetupColumn("Actions");
        //     ImGui::TableHeadersRow();

        //     world.forEachVehicle([&](VehicleId id, vehicle::Vehicle& vehicle) {
        //         ImGui::TableNextRow();

        //         ImGui::TableSetColumnIndex(0);
        //         ImGui::TextUnformatted(vehicle.getDisplayName().c_str());

        //         ImGui::TableSetColumnIndex(1);
        //         ImGui::Text("%d:%d", id.id, id.generation);

        //         ImGui::TableSetColumnIndex(2);
        //         ImGui::Text("%.1f", vehicle.getMaxOperatingSpeedKph());

        //         ImGui::TableSetColumnIndex(3);
        //         ImGui::Text("%.1f", vehicle.getAccelerationKphPerSecond());

        //         ImGui::TableSetColumnIndex(4);
        //         ImGui::Text("%.1f", vehicle.getDecelerationKphPerSecond());

        //         ImGui::TableSetColumnIndex(5);
        //         ImGui::Text("%d", vehicle.getPassengerCapacity());

        //         ImGui::TableSetColumnIndex(6);
        //         const size_t vehicleIndex = static_cast<size_t>(id.id);
        //         std::ostringstream removeLabel;
        //         removeLabel << "Remove##" << id.id << "_" << id.generation;
        //         if (ImGui::SmallButton(removeLabel.str().c_str())) {
        //             world.removeVehicle(vehicleIndex);
        //             LogBuffer::instance().add(
        //                 "Removed vehicle \"" + vehicle.getDisplayName() + "\".");
        //         }
        //         });

        //     ImGui::EndTable();
        // }

        ImGui::Text("Total abstract nodes: %zu", world.getAbstractNodeCount());

        ImGui::End();
    }


    void drawLogPanel() {
        setInitialWindowPosition(ImVec2(10, 525), ImVec2(300, 185));
        ImGui::Begin("Log");

        if (ImGui::Button("Clear Log")) {
            LogBuffer::instance().clear();
        }

        ImGui::Separator();

        if (ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            for (const std::string& line : LogBuffer::instance().getLines()) {
                ImGui::TextUnformatted(line.c_str());
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }
}
