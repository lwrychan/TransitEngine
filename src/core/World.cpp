#include "core/CoreConfig.hpp"
#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"

#include <iostream>
#include <thread>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

namespace core
{
    World::World(const CoreConfig &config)
        : globalConfig(config),
          timestep(1.0 / globalConfig.clockConfig.TARGET_SIMULATION_FPS),
          clock(globalConfig.clockConfig.TARGET_SIMULATION_FPS) {}

    void World::run()
    {
        TimePoint iterationStart = std::chrono::high_resolution_clock::now();
        clock.step();
        while (true)
        {
            TimePoint iterationEnd = std::chrono::high_resolution_clock::now();

            double iterationTime = std::chrono::duration<double>(iterationEnd - iterationStart).count();

            if (iterationTime >= this->timestep)
            {
                if (this->globalConfig.DEBUG_CLOCK)
                {
                    std::cout << "Iteration time: " << iterationTime * 1000 << " ms" << std::endl;
                }

                iterationStart = std::chrono::high_resolution_clock::now();

                // Check for simulation processing time
                TimePoint stepStart = std::chrono::high_resolution_clock::now();

                // Run step for all simulation modules here
                clock.step();








                
                TimePoint stepEnd = std::chrono::high_resolution_clock::now();
            }
            else
            {

                std::this_thread::sleep_for(std::chrono::duration<double>((this->timestep - iterationTime) - (this->globalConfig.clockConfig.THREAD_SLEEP_VARIATION_ADJUSTMENT * 1e-3)));
            }

            if (this->globalConfig.DEBUG_CLOCK)
            {
                // Check for 1 ms deviation from expected timestep and log a warning if the simulation is lagging behind
                if (std::abs(iterationTime - this->timestep) > this->globalConfig.clockConfig.WARNING_THRESHOLD * 1e-3)
                {
                    std::cout << "WARNING || Simulation step took longer than target timestep. Currently lagging behind by " << (iterationTime - this->timestep) * 1000 << " ms" << std::endl;
                }
            }
        }
    }
}