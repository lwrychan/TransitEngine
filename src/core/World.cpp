#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"
#include "cli/Terminal.hpp"
#include "cli/View.hpp"

#include <functional>
#include <iostream>
#include <thread>
#include <utility>

core::World::World(const CoreConfig& config)
    : globalConfig(config),
      physicalNetwork(abstractNetwork) {
}

network::AbstractNetwork& core::World::getAbstractNetwork()
{
    return this->abstractNetwork;
}

const network::AbstractNetwork& core::World::getAbstractNetwork() const
{
    return this->abstractNetwork;
}

network::PhysicalNetwork& core::World::getPhysicalNetwork()
{
    return this->physicalNetwork;
}

const network::PhysicalNetwork& core::World::getPhysicalNetwork() const
{
    return this->physicalNetwork;
}

network::MapNetwork& core::World::getMapNetwork()
{
    return this->mapNetwork;
}

const network::MapNetwork& core::World::getMapNetwork() const
{
    return this->mapNetwork;
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
    vehicle::Vehicle subway = vehicle::Vehicle("Subway", 70.0, 20.0, 20, 20);
    vehicle::Vehicle commuter = vehicle::Vehicle("Commuter Rail", 120.0, 20.0, 20, 20);

    this->addVehicle(subway);
    this->addVehicle(commuter);
}

void core::World::tick() {
    this->tickCount++;
}

uint64_t core::World::getTickCount() const {
    return this->tickCount;
}

size_t core::World::getVehicleCount() const {
    return this->vehicles.getActiveCount();
}

size_t core::World::getAbstractNodeCount() const {
    return this->nodes.getActiveCount();
}

size_t core::World::getAbstractSegmentCount() const {
    return this->segments.getActiveCount();
}

size_t core::World::getAbstractRouteCount() const {
    return this->routes.getActiveCount();
}

void core::World::forEachVehicle(
    const std::function<void(VehicleId, vehicle::Vehicle&)>& callback) {
    this->vehicles.forEachActive(callback);
}
