#pragma once

#include <string>
#include <vector>

#include "Simulation.hpp"

namespace ui
{
    class LogBuffer
    {
    public:
        static LogBuffer& instance();

        void add(const std::string& message);
        void clear();
        const std::vector<std::string>& getLines() const;

    private:
        static constexpr size_t MAX_LINES = 200;
        std::vector<std::string> lines;
    };

    struct VehicleCreationDraft
    {
        char displayName[64] = "New Vehicle";
        float maxSpeedKph = 80.0f;
        float accelerationKphPerSecond = 15.0f;
        float decelerationKphPerSecond = 15.0f;
        int passengerCapacity = 100;
    };

    VehicleCreationDraft& getVehicleCreationDraft();

    void drawSimulationControls(Simulation& simulation);
    void drawVehicleInspector(Simulation& simulation);
    void drawPerformancePanel(Simulation& simulation);
    void drawWorldSummary(Simulation& simulation);
    void drawPhysicalNetworkView(Simulation& simulation);
    void drawNetworkInspector(Simulation& simulation);
    void drawLogPanel();
}
