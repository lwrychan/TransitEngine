#include "vehicle/rail/RailVehicle.hpp"
#include "vehicle/Vehicle.hpp"

namespace vehicle
{
    namespace rail
    {
        RailVehicle::RailVehicle(double maxOperatingSpeedKph, double accelerationKphPerSecond, double decelerationKphPerSecond, int passengerCapacity)
            : Vehicle(maxOperatingSpeedKph / 3.6,
                      accelerationKphPerSecond / 3.6,
                      decelerationKphPerSecond / 3.6,
                      passengerCapacity)
        {
        }
    }
}