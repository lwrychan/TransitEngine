#pragma once

#include <vector>
#include <chrono>

#include "vehicle/Vehicle.hpp"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Vehicle = vehicle::Vehicle;

namespace core
{
    class Clock
    {
    public:
        Clock(double timestep);
        void step();
        double getSimulationDelta();

    private:
        double deltaTime;
        TimePoint lastTimestamp;
        TimePoint currentTimestamp;
        std::vector<Vehicle> vehicles;
    };
}