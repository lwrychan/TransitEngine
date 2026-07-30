#pragma once

#include "core/SegmentId.hpp"
#include "core/RouteId.hpp"

namespace vehicle
{
    class Vehicle
    {
    public:
        Vehicle(double maxOperatingSpeedKph, double accelerationKphPerSecond, double decelerationKphPerSecond, int passengerCapacity);

        void update(double timestep);

        const double getMaxOperatingSpeedMs();
        // Returns configured maximum speed in meters/second (m/s)

        const double getMaxOperatingSpeedKph();
        // Returns maximum speed in kilometers/hour (km/h)

        const double getAccelerationMs2();
        // Returns configured acceleration in meters/second squared (m/s^2)

        const double getDecelerationMs2();
        // Returns configured deceleration in meters/second squared (m/s^2)

        const double getAccelerationKphPerSecond();
        // Returns configured acceleration in meters/second squared (km/h/s)

        const double getDecelerationKphPerSecond();
        // Returns configured deceleration in meters/second squared (km/h/s)

        const int getPassengerCapacity();
        // Returns configured passenger capacity (number of passengers)

    protected:
        double setMaxOperatingSpeed(double speedKph);
        double setAcceleration(double accelerationKphPerSecond);
        double setDeceleration(double decelerationKphPerSecond);
        int setPassengerCapacity(int capacity);

        void setRoute(core::RouteId route);
        void setSegment(core::SegmentId route);

    private:
        double maxOperatingSpeed;
        double acceleration;
        double deceleration;
        int passengerCapacity;

        core::RouteId assignedRouteId;
        core::SegmentId currentSegmentId;

        double segmentProgress = 0.0;
    };
}