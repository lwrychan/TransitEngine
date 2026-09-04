#include "core/World.hpp"
#include "network/AbstractNode.hpp"
#include "network/AbstractRoute.hpp"
#include "vehicle/Vehicle.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
bool isClosedRoute(const std::vector<AbstractNodeId>& nodes)
{
  return nodes.size() >= 3 && nodes.front().id == nodes.back().id &&
         nodes.front().generation == nodes.back().generation;
}

struct RouteControlPoint
{
  double distance = 0.0;
  double progress = 0.0;
  double dwellSeconds = 0.0;
  bool reversesAtEnd = false;
};
} // namespace

core::World::World(const CoreConfig& config)
    : globalConfig(config), physicalNetwork(abstractNetwork)
{
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
  for (AbstractNodeId node : this->routes.get(route).getNodes())
  {
    const bool alreadyAdded =
        std::any_of(stops.begin(), stops.end(), [node](const vehicle::RouteStop& stop)
                    { return stop.node.id == node.id && stop.node.generation == node.generation; });
    if (!alreadyAdded)
    {
      stops.push_back({node, 20.0});
    }
  }
  this->vehicles.get(vehicle).assignRoute(route, std::move(stops));
}

void core::World::clearVehicleRoute(VehicleId vehicle)
{
  this->vehicles.get(vehicle).clearRoute();
}

void core::World::setVehicleRouteStops(VehicleId vehicle, std::vector<vehicle::RouteStop> stops)
{
  this->vehicles.get(vehicle).setStops(std::move(stops));
}

std::optional<core::PhysicalVehiclePose> core::World::getVehiclePose(VehicleId vehicle) const
{
  const vehicle::Vehicle& currentVehicle = this->vehicles.get(vehicle);
  if (!currentVehicle.getAssignedRoute().has_value())
  {
    return std::nullopt;
  }

  const network::AbstractRoute& route = this->routes.get(*currentVehicle.getAssignedRoute());
  const std::vector<AbstractNodeId>& nodesOnRoute = route.getNodes();
  if (nodesOnRoute.size() < 2)
  {
    return std::nullopt;
  }

  const network::PhysicalRouteGeometry* geometry =
      physicalNetwork.getRouteGeometry(*currentVehicle.getAssignedRoute());
  if (geometry == nullptr)
  {
    return std::nullopt;
  }
  const std::vector<double> nodeDistances =
      getRouteNodeDistances(*currentVehicle.getAssignedRoute(), nodesOnRoute);
  const double totalLength = nodeDistances.back();
  if (totalLength <= 0.0)
  {
    return std::nullopt;
  }

  const std::optional<network::RouteGeometryPose> pose =
      network::routeGeometryPoseAtDistance(*geometry, currentVehicle.getRouteProgressMeters());
  if (!pose.has_value())
  {
    return std::nullopt;
  }
  const double direction = static_cast<double>(currentVehicle.getRouteDirection());
  return PhysicalVehiclePose{.coordinate = pose->coordinate,
                             .direction = {pose->direction.x * direction,
                                           pose->direction.y * direction,
                                           pose->direction.z * direction}};
}

void core::World::addNode(network::NodeType nodeType,
                          const network::PhysicalCoordinate& nodeCoordinate,
                          const std::string& nodeName)
{
  network::AbstractNode newNode(nodeName);
  AbstractNodeId newNodeId = this->nodes.add(std::move(newNode));

  if (this->physicalNodes.size() != this->nodes.getSlotCount())
  {
    this->physicalNodes.resize(this->nodes.getSlotCount());
  }

  this->physicalNodes[newNodeId.id] =
      std::move(network::PhysicalNode(newNodeId, nodeType, nodeCoordinate));
}

bool core::World::updateNode(AbstractNodeId node, network::NodeType nodeType,
                             const network::PhysicalCoordinate& nodeCoordinate,
                             const std::string& nodeName)
{
  if (node.id < 0 || static_cast<size_t>(node.id) >= this->physicalNodes.size() ||
      this->hasNodeAt(nodeCoordinate))
  {
    const network::PhysicalCoordinate current = this->getPhysicalNode(node).getCoordinate();
    if (current.x != nodeCoordinate.x || current.y != nodeCoordinate.y ||
        current.z != nodeCoordinate.z)
    {
      return false;
    }
  }

  this->getNode(node).setName(nodeName);
  this->getPhysicalNode(node).setType(nodeType);
  this->getPhysicalNode(node).setCoordinate(nodeCoordinate);
  this->physicalNetwork.updateRouteAnchor(node, nodeCoordinate);
  return true;
}

AbstractRouteId core::World::addRoute(const std::string& routeName,
                                      const std::vector<AbstractNodeId>& nodeIds,
                                      const network::RouteColor& color)
{
  const AbstractRouteId route = this->routes.add(network::AbstractRoute(routeName, nodeIds, color));
  this->physicalNetwork.setRouteGeometry(createDefaultRouteGeometry(route, nodeIds));
  return route;
}

bool core::World::updateRoute(AbstractRouteId route, const std::string& routeName,
                              const std::vector<AbstractNodeId>& nodeIds,
                              const network::RouteColor& color)
{
  if (route.id < 0 || nodeIds.size() < 2)
  {
    return false;
  }

  network::AbstractRoute& existingRoute = this->routes.get(route);
  const std::vector<AbstractNodeId>& previousNodes = existingRoute.getNodes();
  const bool topologyChanged =
      previousNodes.size() != nodeIds.size() ||
      !std::equal(previousNodes.begin(), previousNodes.end(), nodeIds.begin(),
                  [](AbstractNodeId left, AbstractNodeId right)
                  { return left.id != right.id || left.generation != right.generation; });
  existingRoute.setName(routeName);
  existingRoute.setNodes(nodeIds);
  existingRoute.setColor(color);
  if (topologyChanged)
  {
    this->physicalNetwork.setRouteGeometry(createDefaultRouteGeometry(route, nodeIds));
  }
  return true;
}

const network::PhysicalRouteGeometry* core::World::getRouteGeometry(AbstractRouteId route) const
{
  return physicalNetwork.getRouteGeometry(route);
}

bool core::World::insertRouteGeometryNode(AbstractRouteId route, size_t spanIndex,
                                          const network::PhysicalCoordinate& coordinate)
{
  network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  if (geometry == nullptr || spanIndex >= geometry->spans.size())
  {
    return false;
  }
  const network::PhysicalRouteGeometrySpan span = geometry->spans[spanIndex];
  geometry->nodes.insert(geometry->nodes.begin() + static_cast<std::ptrdiff_t>(spanIndex + 1),
                         {.coordinate = coordinate});
  geometry->spans.insert(geometry->spans.begin() + static_cast<std::ptrdiff_t>(spanIndex + 1),
                         span);
  return true;
}

bool core::World::moveRouteGeometryNode(AbstractRouteId route, size_t nodeIndex,
                                        const network::PhysicalCoordinate& coordinate)
{
  network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  if (geometry == nullptr || nodeIndex >= geometry->nodes.size() ||
      geometry->nodes[nodeIndex].anchorNode.has_value())
  {
    return false;
  }
  geometry->nodes[nodeIndex].coordinate = coordinate;
  return true;
}

bool core::World::removeRouteGeometryNode(AbstractRouteId route, size_t nodeIndex)
{
  network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  if (geometry == nullptr || nodeIndex == 0 || nodeIndex + 1 >= geometry->nodes.size() ||
      geometry->nodes[nodeIndex].anchorNode.has_value())
  {
    return false;
  }
  geometry->nodes.erase(geometry->nodes.begin() + static_cast<std::ptrdiff_t>(nodeIndex));
  geometry->spans.erase(geometry->spans.begin() + static_cast<std::ptrdiff_t>(nodeIndex));
  return true;
}

bool core::World::setRouteGeometrySpanInterpolation(AbstractRouteId route, size_t spanIndex,
                                                    network::GeometryInterpolation interpolation)
{
  network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  if (geometry == nullptr || spanIndex >= geometry->spans.size())
  {
    return false;
  }
  geometry->spans[spanIndex].interpolation = interpolation;
  return true;
}

bool core::World::setRouteGeometrySpanSpeedLimit(AbstractRouteId route, size_t spanIndex,
                                                 double maximumSpeedKph)
{
  network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  if (geometry == nullptr || spanIndex >= geometry->spans.size() ||
      !std::isfinite(maximumSpeedKph) || maximumSpeedKph <= 0.0)
  {
    return false;
  }
  geometry->spans[spanIndex].maximumSpeedKph = maximumSpeedKph;
  return true;
}

void core::World::removeNode(AbstractNodeId node)
{
  if (node.id < 0 || static_cast<size_t>(node.id) >= this->physicalNodes.size())
  {
    return;
  }

  this->nodes.clear(static_cast<size_t>(node.id));
  this->physicalNodes[static_cast<size_t>(node.id)] = network::PhysicalNode();
  if (this->activeNode.has_value() && this->activeNode->id == node.id &&
      this->activeNode->generation == node.generation)
  {
    this->activeNode.reset();
  }
}

void core::World::removeRoute(AbstractRouteId route)
{
  if (route.id < 0)
  {
    return;
  }

  this->vehicles.forEachActive(
      [&](VehicleId, vehicle::Vehicle& vehicle)
      {
        const std::optional<AbstractRouteId>& assignedRoute = vehicle.getAssignedRoute();
        if (assignedRoute.has_value() && assignedRoute->id == route.id &&
            assignedRoute->generation == route.generation)
        {
          vehicle.clearRoute();
        }
      });
  this->physicalNetwork.removeRouteGeometry(route);
  this->routes.clear(static_cast<size_t>(route.id));
  if (this->activeRoute.has_value() && this->activeRoute->id == route.id &&
      this->activeRoute->generation == route.generation)
  {
    this->activeRoute.reset();
  }
}

bool core::World::hasNodeAt(const network::PhysicalCoordinate& nodeCoordinate) const
{
  for (const network::PhysicalNode& physicalNode : this->physicalNodes)
  {
    if (physicalNode.hasCoordinate(nodeCoordinate))
    {
      return true;
    }
  }
  return false;
}

void core::World::setActiveNodeAt(const network::PhysicalCoordinate& nodeCoordinate)
{
  clearActiveSelection();
  for (const network::PhysicalNode& physicalNode : this->physicalNodes)
  {
    if (physicalNode.hasCoordinate(nodeCoordinate))
    {
      activeNode = physicalNode.getNodeId();
      return;
    }
  }
}

void core::World::clearActiveSelection()
{
  activeNode.reset();
  activeRoute.reset();
}

network::AbstractNode& core::World::getNode(AbstractNodeId node)
{
  return this->nodes.get(node);
}

const network::AbstractNode& core::World::getNode(AbstractNodeId node) const
{
  return this->nodes.get(node);
}

network::PhysicalNode& core::World::getPhysicalNode(AbstractNodeId node)
{
  return this->physicalNodes.at(static_cast<size_t>(node.id));
}

const network::PhysicalNode& core::World::getPhysicalNode(AbstractNodeId node) const
{
  return this->physicalNodes.at(static_cast<size_t>(node.id));
}

network::AbstractRoute& core::World::getRoute(AbstractRouteId route)
{
  return this->routes.get(route);
}

const network::AbstractRoute& core::World::getRoute(AbstractRouteId route) const
{
  return this->routes.get(route);
}

void core::World::setActiveRoute(AbstractRouteId route)
{
  this->activeNode.reset();
  this->activeRoute = route;
}

void core::World::tick(double simulationDelta)
{
  this->vehicles.forEachActive(
      [&](VehicleId, vehicle::Vehicle& vehicle)
      {
        if (!vehicle.getAssignedRoute().has_value())
        {
          return;
        }

        const network::AbstractRoute& route = this->routes.get(*vehicle.getAssignedRoute());
        const std::vector<AbstractNodeId>& nodesOnRoute = route.getNodes();
        if (nodesOnRoute.size() < 2)
        {
          return;
        }
        const bool routeClosed = isClosedRoute(nodesOnRoute);
        if (routeClosed)
        {
          vehicle.setRouteDirection(1);
        }
        const network::PhysicalRouteGeometry* geometry =
            physicalNetwork.getRouteGeometry(*vehicle.getAssignedRoute());
        if (geometry == nullptr)
        {
          return;
        }
        const std::vector<double> nodeDistances =
            getRouteNodeDistances(*vehicle.getAssignedRoute(), nodesOnRoute);
        const double routeLength = nodeDistances.back();
        if (routeLength <= 0.0)
        {
          return;
        }

        double routeProgress = vehicle.getRouteProgressMeters();
        if (routeClosed)
        {
          routeProgress = std::fmod(routeProgress, routeLength);
          vehicle.setRouteProgressMeters(routeProgress);
        }

        constexpr double arrivalEpsilon = 0.001;
        RouteControlPoint nextControl{.distance = std::numeric_limits<double>::infinity(),
                                      .progress = routeProgress};
        const int direction = vehicle.getRouteDirection();
        for (size_t nodeIndex = 0; nodeIndex < nodesOnRoute.size(); ++nodeIndex)
        {
          const auto stop = std::find_if(
              vehicle.getStops().begin(), vehicle.getStops().end(),
              [node = nodesOnRoute[nodeIndex]](const vehicle::RouteStop& candidate)
              {
                return candidate.node.id == node.id && candidate.node.generation == node.generation;
              });
          if (stop == vehicle.getStops().end())
          {
            continue;
          }
          double distance = direction > 0 ? nodeDistances[nodeIndex] - routeProgress
                                          : routeProgress - nodeDistances[nodeIndex];
          if (routeClosed && distance <= arrivalEpsilon)
          {
            distance += routeLength;
          }
          if (distance > arrivalEpsilon && distance < nextControl.distance)
          {
            nextControl = {.distance = distance,
                           .progress = nodeDistances[nodeIndex],
                           .dwellSeconds = stop->dwellSeconds,
                           .reversesAtEnd =
                               !routeClosed && (direction > 0 ? nodeIndex + 1 == nodesOnRoute.size()
                                                              : nodeIndex == 0)};
          }
        }

        if (!routeClosed && !std::isfinite(nextControl.distance))
        {
          nextControl = {.distance = direction > 0 ? routeLength - routeProgress : routeProgress,
                         .progress = direction > 0 ? routeLength : 0.0,
                         .dwellSeconds = 0.0,
                         .reversesAtEnd = true};
        }

        const bool wasDwelling = vehicle.isDwelling();
        const double speedLimitKph =
            network::routeGeometrySpeedLimitAtDistance(*geometry, vehicle.getRouteProgressMeters());
        const double travelledDistance =
            vehicle.update(simulationDelta, nextControl.distance, speedLimitKph / 3.6);
        const bool reachedControl = !wasDwelling && std::isfinite(nextControl.distance) &&
                                    travelledDistance + arrivalEpsilon >= nextControl.distance;
        if (reachedControl)
        {
          vehicle.setRouteProgressMeters(nextControl.progress);
          if (nextControl.dwellSeconds > 0.0)
          {
            vehicle.beginDwell(nextControl.dwellSeconds);
          }
          if (nextControl.reversesAtEnd)
          {
            vehicle.setRouteDirection(-direction);
          }
        }

        if (routeClosed)
        {
          vehicle.setRouteProgressMeters(std::fmod(vehicle.getRouteProgressMeters(), routeLength));
        }
      });
  this->tickCount++;
}

network::PhysicalRouteGeometry core::World::createDefaultRouteGeometry(
    AbstractRouteId route, const std::vector<AbstractNodeId>& nodesOnRoute) const
{
  network::PhysicalRouteGeometry geometry{.routeId = route};
  geometry.nodes.reserve(nodesOnRoute.size());
  for (AbstractNodeId node : nodesOnRoute)
  {
    geometry.nodes.push_back(
        {.coordinate = getPhysicalNode(node).getCoordinate(), .anchorNode = node});
  }
  if (geometry.nodes.size() >= 2)
  {
    geometry.spans.resize(geometry.nodes.size() - 1);
  }
  return geometry;
}

std::vector<double> core::World::getRouteNodeDistances(
    AbstractRouteId route, const std::vector<AbstractNodeId>& nodesOnRoute) const
{
  const network::PhysicalRouteGeometry* geometry = physicalNetwork.getRouteGeometry(route);
  return geometry == nullptr ? std::vector<double>(nodesOnRoute.size(), 0.0)
                             : network::routeGeometryAnchorDistances(*geometry, nodesOnRoute);
}

uint64_t core::World::getTickCount() const
{
  return this->tickCount;
}

size_t core::World::getVehicleCount() const
{
  return this->vehicles.getActiveCount();
}

size_t core::World::getAbstractNodeCount() const
{
  return this->nodes.getActiveCount();
}

size_t core::World::getAbstractSegmentCount() const
{
  return this->segments.getActiveCount();
}

size_t core::World::getAbstractRouteCount() const
{
  return this->routes.getActiveCount();
}

void core::World::forEachVehicle(const std::function<void(VehicleId, vehicle::Vehicle&)>& callback)
{
  this->vehicles.forEachActive(callback);
}

void core::World::forEachNode(
    const std::function<void(AbstractNodeId, network::AbstractNode&)>& callback)
{
  this->nodes.forEachActive(callback);
}

void core::World::forEachRoute(
    const std::function<void(AbstractRouteId, network::AbstractRoute&)>& callback)
{
  this->routes.forEachActive(callback);
}
