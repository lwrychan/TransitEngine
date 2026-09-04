#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Identifiers.hpp"
#include "network/PhysicalCoordinate.hpp"

namespace network
{
enum class GeometryInterpolation
{
  Linear,
  Bezier
};

// A route anchor mirrors a logical node; other entries are editable physical geometry nodes.
struct PhysicalRouteGeometryNode
{
  PhysicalCoordinate coordinate;
  std::optional<AbstractNodeId> anchorNode;
};

struct PhysicalRouteGeometrySpan
{
  GeometryInterpolation interpolation = GeometryInterpolation::Linear;
  double maximumSpeedKph = 160.0;
};

struct PhysicalRouteGeometry
{
  AbstractRouteId routeId{-1, -1};
  std::vector<PhysicalRouteGeometryNode> nodes;
  std::vector<PhysicalRouteGeometrySpan> spans;
};

struct RouteGeometrySample
{
  PhysicalCoordinate coordinate;
  size_t spanIndex = 0;
};

struct RouteGeometryPose
{
  PhysicalCoordinate coordinate;
  PhysicalCoordinate direction;
  size_t spanIndex = 0;
};

std::vector<RouteGeometrySample> sampleRouteGeometry(const PhysicalRouteGeometry& geometry);
double routeGeometryLength(const PhysicalRouteGeometry& geometry);
std::vector<double> routeGeometryAnchorDistances(const PhysicalRouteGeometry& geometry,
                                                 const std::vector<AbstractNodeId>& routeNodes);
std::optional<RouteGeometryPose> routeGeometryPoseAtDistance(const PhysicalRouteGeometry& geometry,
                                                             double distance);
double routeGeometrySpeedLimitAtDistance(const PhysicalRouteGeometry& geometry, double distance);
} // namespace network
