#include "vehicle/Vehicle.hpp"

namespace core
{
    Vehicle::Vehicle(
        double maxOperatingSpeed,
        double acceleration,
        double deceleration,
        int passengerCapacity)
        : maxOperatingSpeed(maxOperatingSpeed),
          acceleration(acceleration),
          deceleration(deceleration),
          passengerCapacity(passengerCapacity) {}

    void Vehicle::update(double timestep) {};

    const double Vehicle::getMaxOperatingSpeed() { return maxOperatingSpeed; };
    const double Vehicle::getAcceleration() { return acceleration; };
    const double Vehicle::getDeceleration() { return deceleration; };
    const int Vehicle::getPassengerCapacity() { return passengerCapacity; };

    double Vehicle::setMaxOperatingSpeed(double speed) { return maxOperatingSpeed > 0.0 ? maxOperatingSpeed : 0.0; };
    double Vehicle::setAcceleration(double acceleration) { return acceleration > 0.0 ? acceleration : 0.0; };
    double Vehicle::setDeceleration(double deceleration) { return deceleration > 0.0 ? deceleration : 0.0; };
    int Vehicle::setPassengerCapacity(int capacity) { return capacity > 0 ? capacity : 0; };
}