#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "Clock.hpp"
#include "network/AbstractNetwork.hpp"
#include "network/MapNetwork.hpp"
#include "network/PhysicalNetwork.hpp"
#include "cli/View.hpp"
#include "Identifiers.hpp"

#include "SlotMap.hpp"

using View = cli::View;

namespace core
{
    class World
    {
    public:
        World(const CoreConfig& config);

        void addVehicle(Vehicle vehicle);
        void removeVehicle(size_t vehicleIndex);

        network::AbstractNetwork& getAbstractNetwork();
        const network::AbstractNetwork& getAbstractNetwork() const;
        network::PhysicalNetwork& getPhysicalNetwork();
        const network::PhysicalNetwork& getPhysicalNetwork() const;
        network::MapNetwork& getMapNetwork();
        const network::MapNetwork& getMapNetwork() const;

        void setup();

        void tick();

        uint64_t getTickCount() const;
        size_t getVehicleCount() const;
        size_t getAbstractNodeCount() const;
        size_t getAbstractSegmentCount() const;
        size_t getAbstractRouteCount() const;

        void forEachVehicle(const std::function<void(VehicleId, vehicle::Vehicle&)>& callback);

    private:
        CoreConfig globalConfig;
        network::AbstractNetwork abstractNetwork;
        network::PhysicalNetwork physicalNetwork;
        network::MapNetwork mapNetwork;
        SlotMap<vehicle::Vehicle, VehicleId> vehicles;

        SlotMap<network::AbstractNode, AbstractNodeId> nodes;
        SlotMap<network::AbstractSegment, AbstractSegmentId> segments;
        SlotMap<network::AbstractRoute, AbstractRouteId> routes;

        uint64_t tickCount = 0;
    };
}
