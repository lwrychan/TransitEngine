#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Identifiers.hpp"

namespace vehicle
{
    struct RouteStop
    {
        AbstractNodeId node;
        double dwellSeconds = 20.0;
    };

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

        double update(double timestep, double distanceToControlPoint);

        void assignRoute(AbstractRouteId route, std::vector<RouteStop> stops);
        void clearRoute();
        const std::optional<AbstractRouteId>& getAssignedRoute() const;
        const std::vector<RouteStop>& getStops() const;
        void setStops(std::vector<RouteStop> stops);
        void beginDwell(double seconds);
        bool isDwelling() const;
        double getRemainingDwellSeconds() const;
        double getRouteProgressMeters() const;
        void setRouteProgressMeters(double progressMeters);
        int getRouteDirection() const;
        void setRouteDirection(int direction);

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

        double getCurrentSpeedKph() const;

    protected:
        double setMaxOperatingSpeed(double speedKph);
        double setAcceleration(double accelerationKphPerSecond);
        double setDeceleration(double decelerationKphPerSecond);
        int setPassengerCapacity(int capacity);

        void setSegment(AbstractSegmentId segment);

    private:
        std::string displayName;
        double maxOperatingSpeed;
        double acceleration;
        double deceleration;
        int passengerCapacity;

        std::optional<AbstractRouteId> assignedRouteId;
        std::vector<RouteStop> stops;
        AbstractSegmentId currentSegmentId;

        double routeProgressMeters = 0.0;
        int routeDirection = 1;
        double currentSpeed = 0.0;
        double remainingDwellSeconds = 0.0;
    };
}
