#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"
#include "network/AbstractNode.hpp"
#include "network/AbstractRoute.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    bool isClosedRoute(const std::vector<AbstractNodeId>& nodes) {
        return nodes.size() >= 3
            && nodes.front().id == nodes.back().id
            && nodes.front().generation == nodes.back().generation;
    }

    struct RouteControlPoint
    {
        double distance = 0.0;
        double progress = 0.0;
        double dwellSeconds = 0.0;
        bool reversesAtEnd = false;
    };
}

core::World::World(const CoreConfig& config)
    : globalConfig(config),
    physicalNetwork(abstractNetwork) {
}

network::AbstractNetwork& core::World::getAbstractNetwork()
{
    return this->abstractNetwork;
}

const network::AbstractNetwork& core::World::getAbstractNetwork() const
{
    return this->abstractNetwork;
}

network::PhysicalNetwork& core::World::getPhysicalNetwork()
{
    return this->physicalNetwork;
}

const network::PhysicalNetwork& core::World::getPhysicalNetwork() const
{
    return this->physicalNetwork;
}

network::MapNetwork& core::World::getMapNetwork()
{
    return this->mapNetwork;
}

const network::MapNetwork& core::World::getMapNetwork() const
{
    return this->mapNetwork;
}

VehicleId core::World::addVehicle(vehicle::Vehicle vehicle)
{
    return this->vehicles.add(std::move(vehicle));
}

void core::World::removeVehicle(std::size_t vehicleIndex)
{
    this->vehicles.clear(vehicleIndex);
}

void core::World::assignVehicleToRoute(VehicleId vehicle, AbstractRouteId route)
{
    std::vector<vehicle::RouteStop> stops;
    for (AbstractNodeId node : this->routes.get(route).getNodes()) {
        const bool alreadyAdded = std::any_of(stops.begin(), stops.end(),
            [node](const vehicle::RouteStop& stop) {
                return stop.node.id == node.id && stop.node.generation == node.generation;
            });
        if (!alreadyAdded) {
            stops.push_back({node, 20.0});
        }
    }
    this->vehicles.get(vehicle).assignRoute(route, std::move(stops));
}

void core::World::clearVehicleRoute(VehicleId vehicle)
{
    this->vehicles.get(vehicle).clearRoute();
}

void core::World::setVehicleRouteStops(
    VehicleId vehicle,
    std::vector<vehicle::RouteStop> stops)
{
    this->vehicles.get(vehicle).setStops(std::move(stops));
}

std::optional<core::PhysicalVehiclePose> core::World::getVehiclePose(VehicleId vehicle) const
{
    const vehicle::Vehicle& currentVehicle = this->vehicles.get(vehicle);
    if (!currentVehicle.getAssignedRoute().has_value()) {
        return std::nullopt;
    }

    const network::AbstractRoute& route = this->routes.get(*currentVehicle.getAssignedRoute());
    const std::vector<AbstractNodeId>& nodesOnRoute = route.getNodes();
    if (nodesOnRoute.size() < 2) {
        return std::nullopt;
    }

    const std::vector<double> nodeDistances = getRouteNodeDistances(nodesOnRoute);
    const double totalLength = nodeDistances.back();
    if (totalLength <= 0.0) {
        return std::nullopt;
    }

    double remainingDistance = currentVehicle.getRouteProgressMeters();
    for (size_t index = 1; index < nodesOnRoute.size(); ++index) {
        const network::PhysicalCoordinate& start = getPhysicalNode(nodesOnRoute[index - 1]).getCoordinate();
        const network::PhysicalCoordinate& end = getPhysicalNode(nodesOnRoute[index]).getCoordinate();
        const double deltaX = end.x - start.x;
        const double deltaY = end.y - start.y;
        const double segmentLength = nodeDistances[index] - nodeDistances[index - 1];
        if (segmentLength <= 0.0) {
            continue;
        }
        if (remainingDistance <= segmentLength || index + 1 == nodesOnRoute.size()) {
            const double fraction = std::clamp(remainingDistance / segmentLength, 0.0, 1.0);
            return PhysicalVehiclePose{
                .coordinate = {start.x + deltaX * fraction, start.y + deltaY * fraction, start.z + (end.z - start.z) * fraction},
                .direction = {
                    (deltaX / segmentLength) * currentVehicle.getRouteDirection(),
                    (deltaY / segmentLength) * currentVehicle.getRouteDirection(),
                    0.0}
            };
        }
        remainingDistance -= segmentLength;
    }
    return std::nullopt;
}

void core::World::addNode(
    network::NodeType nodeType,
    const network::PhysicalCoordinate& nodeCoordinate,
    const std::string& nodeName) {
    network::AbstractNode newNode(nodeName);
    AbstractNodeId newNodeId = this->nodes.add(std::move(newNode));

    if (this->physicalNodes.size() != this->nodes.getSlotCount()) {
        this->physicalNodes.resize(this->nodes.getSlotCount());
    }

    this->physicalNodes[newNodeId.id] = std::move(
        network::PhysicalNode(newNodeId, nodeType, nodeCoordinate));
}

bool core::World::updateNode(
    AbstractNodeId node,
    network::NodeType nodeType,
    const network::PhysicalCoordinate& nodeCoordinate,
    const std::string& nodeName) {
    if (node.id < 0 || static_cast<size_t>(node.id) >= this->physicalNodes.size()
        || this->hasNodeAt(nodeCoordinate)) {
        const network::PhysicalCoordinate current =
            this->getPhysicalNode(node).getCoordinate();
        if (current.x != nodeCoordinate.x
            || current.y != nodeCoordinate.y
            || current.z != nodeCoordinate.z) {
            return false;
        }
    }

    this->getNode(node).setName(nodeName);
    this->getPhysicalNode(node).setType(nodeType);
    this->getPhysicalNode(node).setCoordinate(nodeCoordinate);
    return true;
}

AbstractRouteId core::World::addRoute(
    const std::string& routeName,
    const std::vector<AbstractNodeId>& nodeIds,
    const network::RouteColor& color) {
    return this->routes.add(network::AbstractRoute(routeName, nodeIds, color));
}

bool core::World::updateRoute(
    AbstractRouteId route,
    const std::string& routeName,
    const std::vector<AbstractNodeId>& nodeIds,
    const network::RouteColor& color) {
    if (route.id < 0 || nodeIds.size() < 2) {
        return false;
    }

    network::AbstractRoute& existingRoute = this->routes.get(route);
    existingRoute.setName(routeName);
    existingRoute.setNodes(nodeIds);
    existingRoute.setColor(color);
    return true;
}

void core::World::removeNode(AbstractNodeId node) {
    if (node.id < 0 || static_cast<size_t>(node.id) >= this->physicalNodes.size()) {
        return;
    }

    this->nodes.clear(static_cast<size_t>(node.id));
    this->physicalNodes[static_cast<size_t>(node.id)] = network::PhysicalNode();
    if (this->activeNode.has_value()
        && this->activeNode->id == node.id
        && this->activeNode->generation == node.generation) {
        this->activeNode.reset();
    }
}

void core::World::removeRoute(AbstractRouteId route) {
    if (route.id < 0) {
        return;
    }

    this->vehicles.forEachActive([&](VehicleId, vehicle::Vehicle& vehicle) {
        const std::optional<AbstractRouteId>& assignedRoute = vehicle.getAssignedRoute();
        if (assignedRoute.has_value()
            && assignedRoute->id == route.id
            && assignedRoute->generation == route.generation) {
            vehicle.clearRoute();
        }
    });
    this->routes.clear(static_cast<size_t>(route.id));
    if (this->activeRoute.has_value()
        && this->activeRoute->id == route.id
        && this->activeRoute->generation == route.generation) {
        this->activeRoute.reset();
    }
}

bool core::World::hasNodeAt(
    const network::PhysicalCoordinate& nodeCoordinate) const {
    for (const network::PhysicalNode& physicalNode : this->physicalNodes) {
        if (physicalNode.hasCoordinate(nodeCoordinate)) {
            return true;
        }
    }
    return false;
}

void core::World::setActiveNodeAt(
    const network::PhysicalCoordinate& nodeCoordinate) {
    activeNode.reset();
    activeRoute.reset();
    for (const network::PhysicalNode& physicalNode : this->physicalNodes) {
        if (physicalNode.hasCoordinate(nodeCoordinate)) {
            activeNode = physicalNode.getNodeId();
            return;
        }
    }
}

network::AbstractNode& core::World::getNode(AbstractNodeId node) {
    return this->nodes.get(node);
}

const network::AbstractNode& core::World::getNode(AbstractNodeId node) const {
    return this->nodes.get(node);
}

network::PhysicalNode& core::World::getPhysicalNode(AbstractNodeId node) {
    return this->physicalNodes.at(static_cast<size_t>(node.id));
}

const network::PhysicalNode& core::World::getPhysicalNode(AbstractNodeId node) const {
    return this->physicalNodes.at(static_cast<size_t>(node.id));
}

network::AbstractRoute& core::World::getRoute(AbstractRouteId route) {
    return this->routes.get(route);
}

const network::AbstractRoute& core::World::getRoute(AbstractRouteId route) const {
    return this->routes.get(route);
}

void core::World::setActiveRoute(AbstractRouteId route) {
    this->activeNode.reset();
    this->activeRoute = route;
}



void core::World::tick(double simulationDelta) {
    this->vehicles.forEachActive([&](VehicleId, vehicle::Vehicle& vehicle) {
        if (!vehicle.getAssignedRoute().has_value()) {
            return;
        }

        const network::AbstractRoute& route = this->routes.get(*vehicle.getAssignedRoute());
        const std::vector<AbstractNodeId>& nodesOnRoute = route.getNodes();
        if (nodesOnRoute.size() < 2) {
            return;
        }
        const bool routeClosed = isClosedRoute(nodesOnRoute);
        if (routeClosed) {
            vehicle.setRouteDirection(1);
        }
        const std::vector<double> nodeDistances = getRouteNodeDistances(nodesOnRoute);
        const double routeLength = nodeDistances.back();
        if (routeLength <= 0.0) {
            return;
        }

        double routeProgress = vehicle.getRouteProgressMeters();
        if (routeClosed) {
            routeProgress = std::fmod(routeProgress, routeLength);
            vehicle.setRouteProgressMeters(routeProgress);
        }

        constexpr double arrivalEpsilon = 0.001;
        RouteControlPoint nextControl{
            .distance = std::numeric_limits<double>::infinity(),
            .progress = routeProgress};
        const int direction = vehicle.getRouteDirection();
        for (size_t nodeIndex = 0; nodeIndex < nodesOnRoute.size(); ++nodeIndex) {
            const auto stop = std::find_if(vehicle.getStops().begin(), vehicle.getStops().end(),
                [node = nodesOnRoute[nodeIndex]](const vehicle::RouteStop& candidate) {
                    return candidate.node.id == node.id && candidate.node.generation == node.generation;
                });
            if (stop == vehicle.getStops().end()) {
                continue;
            }
            double distance = direction > 0
                ? nodeDistances[nodeIndex] - routeProgress
                : routeProgress - nodeDistances[nodeIndex];
            if (routeClosed && distance <= arrivalEpsilon) {
                distance += routeLength;
            }
            if (distance > arrivalEpsilon && distance < nextControl.distance) {
                nextControl = {
                    .distance = distance,
                    .progress = nodeDistances[nodeIndex],
                    .dwellSeconds = stop->dwellSeconds,
                    .reversesAtEnd = !routeClosed
                        && (direction > 0
                            ? nodeIndex + 1 == nodesOnRoute.size()
                            : nodeIndex == 0)};
            }
        }

        if (!routeClosed && !std::isfinite(nextControl.distance)) {
            nextControl = {
                .distance = direction > 0 ? routeLength - routeProgress : routeProgress,
                .progress = direction > 0 ? routeLength : 0.0,
                .dwellSeconds = 0.0,
                .reversesAtEnd = true};
        }

        const bool wasDwelling = vehicle.isDwelling();
        const double travelledDistance = vehicle.update(simulationDelta, nextControl.distance);
        const bool reachedControl = !wasDwelling
            && std::isfinite(nextControl.distance)
            && travelledDistance + arrivalEpsilon >= nextControl.distance;
        if (reachedControl) {
            vehicle.setRouteProgressMeters(nextControl.progress);
            if (nextControl.dwellSeconds > 0.0) {
                vehicle.beginDwell(nextControl.dwellSeconds);
            }
            if (nextControl.reversesAtEnd) {
                vehicle.setRouteDirection(-direction);
            }
        }

        if (routeClosed) {
            vehicle.setRouteProgressMeters(std::fmod(vehicle.getRouteProgressMeters(), routeLength));
        }
    });
    this->tickCount++;
}

std::vector<double> core::World::getRouteNodeDistances(
    const std::vector<AbstractNodeId>& nodesOnRoute) const
{
    std::vector<double> distances(nodesOnRoute.size(), 0.0);
    for (size_t index = 1; index < nodesOnRoute.size(); ++index) {
        const network::PhysicalCoordinate& start = getPhysicalNode(nodesOnRoute[index - 1]).getCoordinate();
        const network::PhysicalCoordinate& end = getPhysicalNode(nodesOnRoute[index]).getCoordinate();
        distances[index] = distances[index - 1] + std::hypot(end.x - start.x, end.y - start.y);
    }
    return distances;
}

uint64_t core::World::getTickCount() const {
    return this->tickCount;
}

size_t core::World::getVehicleCount() const {
    return this->vehicles.getActiveCount();
}

size_t core::World::getAbstractNodeCount() const {
    return this->nodes.getActiveCount();
}

size_t core::World::getAbstractSegmentCount() const {
    return this->segments.getActiveCount();
}

size_t core::World::getAbstractRouteCount() const {
    return this->routes.getActiveCount();
}

void core::World::forEachVehicle(
    const std::function<void(VehicleId, vehicle::Vehicle&)>& callback) {
    this->vehicles.forEachActive(callback);
}

void core::World::forEachNode(
    const std::function<void(AbstractNodeId, network::AbstractNode&)>& callback) {
    this->nodes.forEachActive(callback);
}

void core::World::forEachRoute(
    const std::function<void(AbstractRouteId, network::AbstractRoute&)>& callback) {
    this->routes.forEachActive(callback);
}
