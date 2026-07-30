#include "core/Clock.hpp"

namespace core
{
    Clock::Clock(double timestep)
    {
        this->lastTimestamp = std::chrono::high_resolution_clock::now();
        this->currentTimestamp = this->lastTimestamp;
    }

    void Clock::step()
    {
        this->currentTimestamp = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> delta = this->currentTimestamp - this->lastTimestamp;

        this->lastTimestamp = this->currentTimestamp;

        this->deltaTime = std::chrono::duration<double>(delta).count();
    }

    double Clock::getSimulationDelta()
    {
        return this->deltaTime;
    }
}