#include <iostream>
#include <thread>
#include <chrono>

#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "core/Clock.hpp"
#include "vehicle/Vehicle.hpp"
#include "cli/Terminal.hpp"
#include "cli/display/ProgressBar.hpp"

using namespace core;

using ProgressBar = cli::display::ProgressBar;

void simulationStartPrint() {
    cli::Terminal::clearScreen();

    std::cout << "Starting simulation..." << std::endl;
    std::this_thread::sleep_for(std::chrono::duration<double>(1.0));

    cli::Terminal::clearScreen();
}

int main()
{
    return 0;
}