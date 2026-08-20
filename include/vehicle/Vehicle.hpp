#pragma once

#include <string>

#include "Identifiers.hpp"

namespace vehicle
{
    class Vehicle
    {
    public:
        Vehicle() = default;
        Vehicle(
            double maxOperatingSpeedKph,
            double accelerationKphPerSecond,
            double decelerationKphPerSecond,
            int passengerCapacity);
        Vehicle(
            const std::string& displayName,
            double maxOperatingSpeedKph,
            double accelerationKphPerSecond,
            double decelerationKphPerSecond,
            int passengerCapacity);

        void update(double timestep);

        const std::string& getDisplayName() const;

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

        void setRoute(AbstractRouteId route);
        void setSegment(AbstractSegmentId segment);

    private:
        std::string displayName;
        double maxOperatingSpeed;
        double acceleration;
        double deceleration;
        int passengerCapacity;

        AbstractRouteId assignedRouteId;
        AbstractSegmentId currentSegmentId;

        double segmentProgress = 0.0;
    };
}
