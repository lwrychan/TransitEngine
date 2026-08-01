#pragma once

#include "core/CoreConfig.hpp"
#include "core/Clock.hpp"
#include "core/World.hpp"
class Simulation {
public:
    Simulation(const CoreConfig& config);

    void run();

private:
    CoreConfig globalConfig;
    double timestep;
    bool simulationRunning;
    core::Clock clock;
    core::World world;
};