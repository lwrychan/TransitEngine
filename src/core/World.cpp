#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"
#include "cli/Terminal.hpp"
#include "cli/View.hpp"

#include <iostream>
#include <thread>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using View = cli::View;

core::World::World(const CoreConfig& config)
    : globalConfig(config),
    timestep(1.0 / globalConfig.clockConfig.targetSimulationFps),
    clock(globalConfig.clockConfig.targetSimulationFps), view(View(10)) {
}

std::vector<Vehicle>& core::World::getVehicles()
{
    return this->vehicles;
}

network::Network& core::World::getNetwork()
{
    return this->network;
}

int core::World::addVehicle(Vehicle& vehicle)
{
    this->vehicles.push_back(vehicle);
    return int(this->vehicles.size());
}

void core::World::removeVehicle(std::size_t vehicleIndex)
{
    this->vehicles.erase(this->vehicles.begin() + vehicleIndex);
}

void core::World::setup() {
    vehicle::Vehicle subway = vehicle::Vehicle(70.0, 20.0, 20, 20);
    vehicle::Vehicle commuter = vehicle::Vehicle(120.0, 20.0, 20, 20);

    this->addVehicle(subway);
    this->addVehicle(commuter);
}

void core::World::tick() {
    this->clock.step();

    cli::Terminal::clearScreen();

    for (int i = 0; i < this->getVehicles().size(); i++) {
        this->view.setLine(static_cast<size_t>(i), std::string("[") + std::string("VEHICLE #") + std::to_string(i + 1) + "] | Maximum Operating Speed: " + std::to_string((int)this->getVehicles()[i].getMaxOperatingSpeedKph()) + " km/h\n");
    }

    this->view.print();
}


void core::World::run()
{
    TimePoint iterationStart = std::chrono::high_resolution_clock::now();
    this->clock.step();

    // Initial setup
    this->setup();

    while (true)
    {
        TimePoint iterationEnd = std::chrono::high_resolution_clock::now();

        double iterationTime = std::chrono::duration<double>(iterationEnd - iterationStart).count();

        if (iterationTime >= this->timestep)
        {
            if (this->globalConfig.DEBUG_CLOCK)
            {
                std::cout << "Iteration time: " << iterationTime * 1000 << " ms" << std::endl;
            }

            iterationStart = std::chrono::high_resolution_clock::now();

            // Check for simulation processing time
            TimePoint stepStart = std::chrono::high_resolution_clock::now();

            // Run step for all simulation modules here
            this->tick();

            TimePoint stepEnd = std::chrono::high_resolution_clock::now();
        }
        else
        {

            std::this_thread::sleep_for(std::chrono::duration<double>((this->timestep - iterationTime) - (this->globalConfig.clockConfig.THREAD_SLEEP_VARIATION_ADJUSTMENT * 1e-3)));
        }

        if (this->globalConfig.DEBUG_CLOCK)
        {
            // Check for 1 ms deviation from expected timestep and log a warning if the simulation is lagging behind
            if (iterationTime - this->timestep > this->globalConfig.clockConfig.warningThreshold * 1e-3)
            {
                std::cout << "WARNING || Simulation step took longer than target timestep. Currently lagging behind by " << (iterationTime - this->timestep) * 1000 << " ms" << std::endl;
            }
        }
    }
}