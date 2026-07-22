#pragma once

namespace core
{
    class Vehicle
    {
    public:
        Vehicle(double maxOperatingSpeedMs, double acceleration, double deceleration, int passengerCapacity);

        void update(double timestep);

        const double getMaxOperatingSpeed();
        // Returns configured maximum speed in meters/second (m/s)

        const double getMaxOperatingSpeedKmph();
        // Returns maximum speed in kilometers/hour (km/h)

        const double getAcceleration();
        // Returns configured acceleration in meters/second squared (m/s^2)

        const double getDeceleration();
        // Returns configured deceleration in meters/second squared (m/s^2)

        const int getPassengerCapacity();
        // Returns configured passenger capacity (number of passengers)

    protected:
        double setMaxOperatingSpeed(double speedMs);
        double setAcceleration(double acceleration);
        double setDeceleration(double deceleration);
        int setPassengerCapacity(int capacity);

    private:
        double maxOperatingSpeed;
        double acceleration;
        double deceleration;
        int passengerCapacity;
    };
}