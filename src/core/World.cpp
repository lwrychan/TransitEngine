#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"
#include "cli/Terminal.hpp"
#include "cli/View.hpp"

#include <iostream>
#include <thread>
#include <utility>

core::World::World(const CoreConfig& config)
    : globalConfig(config) {
}

network::Network& core::World::getNetwork()
{
    return this->network;
}

void core::World::addVehicle(Vehicle vehicle)
{
    this->vehicles.add(std::move(vehicle));
}

void core::World::removeVehicle(std::size_t vehicleIndex)
{
    this->vehicles.clear(vehicleIndex);
}

void core::World::setup() {
    vehicle::Vehicle subway = vehicle::Vehicle(70.0, 20.0, 20, 20);
    vehicle::Vehicle commuter = vehicle::Vehicle(120.0, 20.0, 20, 20);

    this->addVehicle(subway);
    this->addVehicle(commuter);
}

void core::World::tick() {
    
}