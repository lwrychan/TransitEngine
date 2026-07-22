#pragma once

#include "core/SegmentId.hpp"
#include "core/RouteId.hpp"

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

        void setRoute(RouteId route);
        void setSegment(SegmentId route);

    private:
        double maxOperatingSpeed;
        double acceleration;
        double deceleration;
        int passengerCapacity;

        RouteId assignedRouteId;
        SegmentId currentSegmentId;

        double segmentProgress = 0.0;
    };
}