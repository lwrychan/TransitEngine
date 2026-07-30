#include "vehicle/Vehicle.hpp"

namespace vehicle
{
    Vehicle::Vehicle(
        double maxOperatingSpeedKph,
        double accelerationKphPerSecond,
        double decelerationKphPerSecond,
        int passengerCapacity)
        : maxOperatingSpeed(maxOperatingSpeedKph / 3.6),
          acceleration(accelerationKphPerSecond / 3.6),
          deceleration(decelerationKphPerSecond / 3.6),
          passengerCapacity(passengerCapacity) {}

    void Vehicle::update(double timestep) {};

    const double Vehicle::getMaxOperatingSpeedMs() { return maxOperatingSpeed; };
    const double Vehicle::getMaxOperatingSpeedKph() { return maxOperatingSpeed * 3.6; };
    const double Vehicle::getAccelerationMs2() { return acceleration; };
    const double Vehicle::getDecelerationMs2() { return deceleration; };
    const double Vehicle::getAccelerationKphPerSecond() { return acceleration * 3.6; };
    const double Vehicle::getDecelerationKphPerSecond() { return deceleration * 3.6; };
    const int Vehicle::getPassengerCapacity() { return passengerCapacity; };

    double Vehicle::setMaxOperatingSpeed(double speed) { return maxOperatingSpeed > 0.0 ? maxOperatingSpeed : 0.0; };
    double Vehicle::setAcceleration(double acceleration) { return acceleration > 0.0 ? acceleration : 0.0; };
    double Vehicle::setDeceleration(double deceleration) { return deceleration > 0.0 ? deceleration : 0.0; };
    int Vehicle::setPassengerCapacity(int capacity) { return capacity > 0 ? capacity : 0; };
}