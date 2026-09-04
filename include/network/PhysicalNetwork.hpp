#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "AbstractNetwork.hpp"
#include "Identifiers.hpp"

#include "PhysicalCoordinate.hpp"
#include "RouteGeometry.hpp"

namespace network
{
// The real-world shape of one abstract segment, expressed in meters.
struct PhysicalSegmentGeometry
{
  AbstractSegmentId abstractSegmentId{-1, -1};
  std::vector<PhysicalCoordinate> path;
};

// Physical projection of AbstractNetwork: identical topology plus real track geometry.
class PhysicalNetwork
{
  public:
  explicit PhysicalNetwork(const AbstractNetwork& abstractNetwork)
      : abstractNetwork(&abstractNetwork)
  {
  }

  const AbstractNetwork& getAbstractNetwork() const
  {
    return *abstractNetwork;
  }
  const std::vector<PhysicalSegmentGeometry>& getSegmentGeometries() const
  {
    return segmentGeometries;
  }

  const PhysicalRouteGeometry* getRouteGeometry(AbstractRouteId route) const
  {
    const auto geometry = std::find_if(routeGeometries.begin(), routeGeometries.end(),
                                       [route](const PhysicalRouteGeometry& candidate)
                                       {
                                         return candidate.routeId.id == route.id &&
                                                candidate.routeId.generation == route.generation;
                                       });
    return geometry == routeGeometries.end() ? nullptr : &*geometry;
  }

  PhysicalRouteGeometry* getRouteGeometry(AbstractRouteId route)
  {
    const auto geometry = std::find_if(routeGeometries.begin(), routeGeometries.end(),
                                       [route](const PhysicalRouteGeometry& candidate)
                                       {
                                         return candidate.routeId.id == route.id &&
                                                candidate.routeId.generation == route.generation;
                                       });
    return geometry == routeGeometries.end() ? nullptr : &*geometry;
  }

  void setRouteGeometry(PhysicalRouteGeometry geometry)
  {
    if (PhysicalRouteGeometry* existing = getRouteGeometry(geometry.routeId))
    {
      *existing = std::move(geometry);
      return;
    }
    routeGeometries.push_back(std::move(geometry));
  }

  void removeRouteGeometry(AbstractRouteId route)
  {
    std::erase_if(routeGeometries,
                  [route](const PhysicalRouteGeometry& geometry)
                  {
                    return geometry.routeId.id == route.id &&
                           geometry.routeId.generation == route.generation;
                  });
  }

  void updateRouteAnchor(AbstractNodeId node, const PhysicalCoordinate& coordinate)
  {
    for (PhysicalRouteGeometry& geometry : routeGeometries)
    {
      for (PhysicalRouteGeometryNode& geometryNode : geometry.nodes)
      {
        if (geometryNode.anchorNode.has_value() && geometryNode.anchorNode->id == node.id &&
            geometryNode.anchorNode->generation == node.generation)
        {
          geometryNode.coordinate = coordinate;
        }
      }
    }
  }

  void setSegmentGeometry(AbstractSegmentId segmentId, std::vector<PhysicalCoordinate> path)
  {
    for (PhysicalSegmentGeometry& geometry : segmentGeometries)
    {
      if (geometry.abstractSegmentId.id == segmentId.id &&
          geometry.abstractSegmentId.generation == segmentId.generation)
      {
        geometry.path = std::move(path);
        return;
      }
    }
    segmentGeometries.push_back({segmentId, std::move(path)});
  }

  private:
  const AbstractNetwork* abstractNetwork;
  std::vector<PhysicalSegmentGeometry> segmentGeometries;
  std::vector<PhysicalRouteGeometry> routeGeometries;
};
} // namespace network
