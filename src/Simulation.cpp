#include "Simulation.hpp"
#include "core/World.hpp"
#include "render/Render.hpp"

Simulation::Simulation(const CoreConfig& config)
    : globalConfig(config),
      timestep(1.0 / config.clockConfig.targetSimulationFps),
      simulationRunning(false),
      clock(timestep),
      world(config)
{
}

void Simulation::run() {
    render::Render renderInstance = render::Render();

    TimePoint iterationStart = std::chrono::high_resolution_clock::now();

    // Initial step and setup for World
    this->clock.step();
    this->world.setup();

    // ====================
    // Set up render resources
    renderInstance.setup();
    // ====================
    bool running = true;

    while (running)
    {
        // Update render frame
        if (!renderInstance.update())
        {
            running = false;
            break;
        }

        // SIMULATION LOGIC

        // Loop rate limitation

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
            this->world.tick();

            TimePoint stepEnd = std::chrono::high_resolution_clock::now();
        }
        else
        {

            std::this_thread::sleep_for(std::chrono::duration<double>((this->timestep - iterationTime) - (this->globalConfig.clockConfig.THREAD_SLEEP_VARIATION_ADJUSTMENT * 1e-3)));
        }

        if (this->globalConfig.DEBUG_CLOCK)
        {
            // Check for 1 ms deviation from expected timestep and log a warning if the simulation is lagging behind
            if (iterationTime - this->timestep > this->globalConfig.clockConfig.warningThreshold * 1e-3)
            {
                std::cout << "WARNING || Simulation step took longer than target timestep. Currently lagging behind by " << (iterationTime - this->timestep) * 1000 << " ms" << std::endl;
            }
        }
    }
}




//     // Initial setup
//     this->setup();

//     while (true)
//     {
//         
//     }
