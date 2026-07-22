#pragma once

#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "vehicles/Vehicle.hpp"

namespace core
{
    class World
    {
    public:
        World(const CoreConfig &config);
        void step();

    private:
        double worldTime;
        std::vector<Vehicle> vehicles;
        CoreConfig config;
    };
}