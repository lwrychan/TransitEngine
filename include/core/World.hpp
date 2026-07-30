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

        Clock& getClock() { return clock; };

        std::vector<Vehicle>& getVehicles();

        int addVehicle(Vehicle& vehicle);
        void removeVehicle(size_t vehicleIndex);

        network::Network& getNetwork();

        void setup();

        void tick();

        void run();

        View view;

    private:
        CoreConfig globalConfig;
        Clock clock;
        
        double timestep;
        network::Network network;
        std::vector<Vehicle> vehicles;
    };
}