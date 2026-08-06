#pragma once

#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "Clock.hpp"
#include "network/Network.hpp"
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

        network::Network& getNetwork();

        void setup();

        void tick();

    private:
        CoreConfig globalConfig;
        network::Network network;
        SlotMap<vehicle::Vehicle, VehicleId> vehicles;

        SlotMap<network::Node, NodeId> nodes;
        SlotMap<network::Segment, SegmentId> segments;
        SlotMap<network::Route, RouteId> routes;
    };
}