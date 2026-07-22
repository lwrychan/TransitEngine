#pragma once

#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "Clock.hpp"

namespace core
{
    class World
    {
    public:
        World(const CoreConfig &config);

        Clock &getClock() { return clock; };

        void run();

    private:
        CoreConfig globalConfig;
        Clock clock;
        double timestep;
    };
}