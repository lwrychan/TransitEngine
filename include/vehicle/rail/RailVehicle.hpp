#pragma once

#include "vehicle/Vehicle.hpp"

namespace vehicle
{
namespace rail
{
    class RailVehicle : Vehicle
    {
    public:
        RailVehicle(double maxOperatingSpeedKph, double acceleration, double deceleration,
                    int passengerCapacity);
    };
} // namespace rail

} // namespace vehicle