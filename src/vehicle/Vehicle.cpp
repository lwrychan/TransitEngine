#include "vehicle/Vehicle.hpp"

#include <algorithm>
#include <cmath>

namespace vehicle
{
Vehicle::Vehicle(double maxOperatingSpeedKph, double accelerationKphPerSecond,
                 double decelerationKphPerSecond, int passengerCapacity)
    : Vehicle("Unnamed Vehicle", maxOperatingSpeedKph, accelerationKphPerSecond,
              decelerationKphPerSecond, passengerCapacity)
{
}

Vehicle::Vehicle(const std::string& displayName, double maxOperatingSpeedKph,
                 double accelerationKphPerSecond, double decelerationKphPerSecond,
                 int passengerCapacity)
    : displayName(displayName.empty() ? "Unnamed Vehicle" : displayName),
      maxOperatingSpeed(maxOperatingSpeedKph / 3.6), acceleration(accelerationKphPerSecond / 3.6),
      deceleration(decelerationKphPerSecond / 3.6), passengerCapacity(passengerCapacity)
{
}

double Vehicle::update(double timestep, double distanceToControlPoint)
{
    if (!assignedRouteId.has_value() || timestep <= 0.0)
    {
        return 0.0;
    }
    if (remainingDwellSeconds > 0.0)
    {
        remainingDwellSeconds = std::max(0.0, remainingDwellSeconds - timestep);
        currentSpeed = 0.0;
        return 0.0;
    }

    const double brakingSpeedLimit =
        std::sqrt(std::max(0.0, 2.0 * deceleration * std::max(0.0, distanceToControlPoint)));
    const double targetSpeed = std::min(maxOperatingSpeed, brakingSpeedLimit);
    const double previousSpeed = currentSpeed;
    if (currentSpeed < targetSpeed)
    {
        currentSpeed = std::min(targetSpeed, currentSpeed + acceleration * timestep);
    }
    else
    {
        currentSpeed = std::max(targetSpeed, currentSpeed - deceleration * timestep);
    }
    const double travelledDistance = std::min(std::max(0.0, distanceToControlPoint),
                                              (previousSpeed + currentSpeed) * 0.5 * timestep);
    routeProgressMeters += travelledDistance * routeDirection;
    return travelledDistance;
}

void Vehicle::assignRoute(AbstractRouteId route, std::vector<RouteStop> newStops)
{
    assignedRouteId = route;
    stops = std::move(newStops);
    routeProgressMeters = 0.0;
    routeDirection = 1;
    currentSpeed = 0.0;
}

void Vehicle::clearRoute()
{
    assignedRouteId.reset();
    stops.clear();
    routeProgressMeters = 0.0;
    routeDirection = 1;
    currentSpeed = 0.0;
    remainingDwellSeconds = 0.0;
}

const std::optional<AbstractRouteId>& Vehicle::getAssignedRoute() const
{
    return assignedRouteId;
}

const std::vector<RouteStop>& Vehicle::getStops() const
{
    return stops;
}

void Vehicle::setStops(std::vector<RouteStop> newStops)
{
    stops = std::move(newStops);
}

void Vehicle::beginDwell(double seconds)
{
    remainingDwellSeconds = std::max(0.0, seconds);
    currentSpeed = 0.0;
}

bool Vehicle::isDwelling() const
{
    return remainingDwellSeconds > 0.0;
}

double Vehicle::getRemainingDwellSeconds() const
{
    return remainingDwellSeconds;
}

double Vehicle::getRouteProgressMeters() const
{
    return routeProgressMeters;
}

void Vehicle::setRouteProgressMeters(double progressMeters)
{
    routeProgressMeters = std::max(0.0, progressMeters);
}

int Vehicle::getRouteDirection() const
{
    return routeDirection;
}

void Vehicle::setRouteDirection(int direction)
{
    routeDirection = direction < 0 ? -1 : 1;
}

const std::string& Vehicle::getDisplayName() const
{
    return this->displayName;
}

const double Vehicle::getMaxOperatingSpeedMs()
{
    return maxOperatingSpeed;
};
const double Vehicle::getMaxOperatingSpeedKph()
{
    return maxOperatingSpeed * 3.6;
};
const double Vehicle::getAccelerationMs2()
{
    return acceleration;
};
const double Vehicle::getDecelerationMs2()
{
    return deceleration;
};
const double Vehicle::getAccelerationKphPerSecond()
{
    return acceleration * 3.6;
};
const double Vehicle::getDecelerationKphPerSecond()
{
    return deceleration * 3.6;
};
const int Vehicle::getPassengerCapacity()
{
    return passengerCapacity;
};
double Vehicle::getCurrentSpeedKph() const
{
    return currentSpeed * 3.6;
};

double Vehicle::setMaxOperatingSpeed(double speed)
{
    return maxOperatingSpeed > 0.0 ? maxOperatingSpeed : 0.0;
};
double Vehicle::setAcceleration(double acceleration)
{
    return acceleration > 0.0 ? acceleration : 0.0;
};
double Vehicle::setDeceleration(double deceleration)
{
    return deceleration > 0.0 ? deceleration : 0.0;
};
int Vehicle::setPassengerCapacity(int capacity)
{
    return capacity > 0 ? capacity : 0;
};
} // namespace vehicle
