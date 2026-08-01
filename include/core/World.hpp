#pragma once

#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "Clock.hpp"
#include "network/Network.hpp"
#include "cli/View.hpp"

using View = cli::View;

namespace core
{
    class World
    {
    public:
        World(const CoreConfig& config);

        std::vector<Vehicle>& getVehicles();

        int addVehicle(Vehicle& vehicle);
        void removeVehicle(size_t vehicleIndex);

        network::Network& getNetwork();

        void setup();

        void tick();

    private:
        CoreConfig globalConfig;
        network::Network network;
        std::vector<Vehicle> vehicles;
    };
}