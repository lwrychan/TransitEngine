#include <iostream>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"

using namespace core;

int main()
{
    CoreConfig config;

    World world(config);

    world.run();

    return 0;
}