#include "Simulation.hpp"
#include "core/World.hpp"
#include "render/Render.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

Simulation::Simulation(const CoreConfig& config)
    : globalConfig(config),
    timestep(1.0 / config.clockConfig.targetSimulationFps),
    simulationRunning(false),
    stepOnceRequested(false),
    speedMultiplier(1.0),
    simTime(0.0),
    lastRenderLatencyMs(0.0),
    lastSimulationLatencyMs(0.0),
    lastOverallLatencyMs(0.0),
    world(config)
{
}

core::World& Simulation::getWorld() {
    return this->world;
}

const core::World& Simulation::getWorld() const {
    return this->world;
}

CoreConfig& Simulation::getConfig() {
    return this->globalConfig;
}

const CoreConfig& Simulation::getConfig() const {
    return this->globalConfig;
}

bool Simulation::isRunning() const {
    return this->simulationRunning;
}

void Simulation::setRunning(bool running) {
    this->simulationRunning = running;
}

void Simulation::toggleRunning() {
    this->simulationRunning = !this->simulationRunning;
}

double Simulation::getSpeedMultiplier() const {
    return this->speedMultiplier;
}

void Simulation::setSpeedMultiplier(double multiplier) {
    this->speedMultiplier = multiplier > 0.0 ? multiplier : 1.0;
}

void Simulation::requestStepOnce() {
    this->stepOnceRequested = true;
}

double Simulation::getSimTime() const {
    return this->simTime;
}

double Simulation::getTimestep() const {
    return this->timestep;
}

double Simulation::getLastRenderLatencyMs() const {
    return this->lastRenderLatencyMs;
}

double Simulation::getLastSimulationLatencyMs() const {
    return this->lastSimulationLatencyMs;
}

double Simulation::getLastOverallLatencyMs() const {
    return this->lastOverallLatencyMs;
}

void Simulation::run() {
    render::Render renderInstance = render::Render();

    TimePoint iterationStart = std::chrono::high_resolution_clock::now();

    renderInstance.setup();
    bool running = true;

    while (running)
    {
        if (!renderInstance.update(*this))
        {
            running = false;
            break;
        }

        this->lastRenderLatencyMs = renderInstance.getLastRenderWorkMs();
        this->lastSimulationLatencyMs = 0.0;

        const TimePoint iterationEnd = std::chrono::high_resolution_clock::now();

        const double iterationTime =
            std::chrono::duration<double>(iterationEnd - iterationStart).count();

        const bool shouldTick = this->simulationRunning || this->stepOnceRequested;

        if (iterationTime >= this->timestep && shouldTick)
        {
            if (this->globalConfig.DEBUG_CLOCK)
            {
                std::cout << "Loop interval: " << iterationTime * 1000 << " ms" << std::endl;
            }

            const TimePoint stepStart = std::chrono::high_resolution_clock::now();

            this->world.tick(this->timestep * this->speedMultiplier);
            this->simTime += this->timestep * this->speedMultiplier;
            this->stepOnceRequested = false;

            const TimePoint stepEnd = std::chrono::high_resolution_clock::now();
            this->lastSimulationLatencyMs =
                std::chrono::duration<double>(stepEnd - stepStart).count() * 1000.0;

            iterationStart = std::chrono::high_resolution_clock::now();
        }
        else if (!shouldTick)
        {
            iterationStart = iterationEnd;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(
                (this->timestep - iterationTime)
                - (this->globalConfig.clockConfig.threadSleepVariationAdjustment * 1e-3)));
        }

        this->lastOverallLatencyMs = iterationTime * 1000;

        if (this->globalConfig.DEBUG_CLOCK)
        {
            const double targetMs = this->timestep * 1000.0;
            const double warningThresholdMs = this->globalConfig.clockConfig.warningThreshold;
            if (this->lastOverallLatencyMs > targetMs + warningThresholdMs)
            {
                std::cout << "WARNING || Overall latency exceeded target timestep. Over by "
                    << (this->lastOverallLatencyMs - targetMs) << " ms" << std::endl;
            }
        }
    }

    renderInstance.close();
}
