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
    : globalConfig(config) {
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
}