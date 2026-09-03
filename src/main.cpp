#include <chrono>
#include <iostream>
#include <thread>

#include "Simulation.hpp"
#include "cli/Terminal.hpp"
#include "cli/display/ProgressBar.hpp"
#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"

using namespace core;

using ProgressBar = cli::display::ProgressBar;

void simulationStartPrint()
{
    cli::Terminal::clearScreen();

    std::cout << "Starting simulation..." << std::endl;
    std::this_thread::sleep_for(std::chrono::duration<double>(1.0));

    cli::Terminal::clearScreen();
}

int main()
{
    CoreConfig config;

    config.DEBUG_CLOCK = false;

    // // Run the synchronized simulation loop inside the world.
    // World world(config);

    Simulation sim = Simulation(config);

    sim.run();

    return 0;
}
