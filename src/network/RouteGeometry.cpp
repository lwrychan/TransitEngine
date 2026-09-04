#include "network/RouteGeometry.hpp"

#include <algorithm>
#include <cmath>

namespace network
{
namespace
{
  PhysicalCoordinate subtract(const PhysicalCoordinate& left, const PhysicalCoordinate& right)
  {
    return {left.x - right.x, left.y - right.y, left.z - right.z};
  }

  PhysicalCoordinate add(const PhysicalCoordinate& left, const PhysicalCoordinate& right)
  {
    return {left.x + right.x, left.y + right.y, left.z + right.z};
  }

  PhysicalCoordinate multiply(const PhysicalCoordinate& coordinate, double factor)
  {
    return {coordinate.x * factor, coordinate.y * factor, coordinate.z * factor};
  }

  double distance(const PhysicalCoordinate& start, const PhysicalCoordinate& end)
  {
    return std::hypot(end.x - start.x, end.y - start.y, end.z - start.z);
  }

  PhysicalCoordinate evaluateBezier(const PhysicalRouteGeometry& geometry, size_t spanIndex,
                                    double t)
  {
    const PhysicalCoordinate& start = geometry.nodes[spanIndex].coordinate;
    const PhysicalCoordinate& end = geometry.nodes[spanIndex + 1].coordinate;
    const PhysicalCoordinate& before =
        geometry.nodes[spanIndex == 0 ? spanIndex : spanIndex - 1].coordinate;
    const PhysicalCoordinate& after =
        geometry.nodes[std::min(spanIndex + 2, geometry.nodes.size() - 1)].coordinate;
    const PhysicalCoordinate startTangent = multiply(subtract(end, before), 0.5);
    const PhysicalCoordinate endTangent = multiply(subtract(after, start), 0.5);
    const double t2 = t * t;
    const double t3 = t2 * t;
    return add(
        add(multiply(start, 2.0 * t3 - 3.0 * t2 + 1.0), multiply(startTangent, t3 - 2.0 * t2 + t)),
        add(multiply(end, -2.0 * t3 + 3.0 * t2), multiply(endTangent, t3 - t2)));
  }

  bool sameNode(AbstractNodeId left, AbstractNodeId right)
  {
    return left.id == right.id && left.generation == right.generation;
  }
} // namespace

std::vector<RouteGeometrySample> sampleRouteGeometry(const PhysicalRouteGeometry& geometry)
{
  std::vector<RouteGeometrySample> samples;
  if (geometry.nodes.empty())
  {
    return samples;
  }

  samples.push_back({geometry.nodes.front().coordinate, 0});
  for (size_t spanIndex = 0; spanIndex < geometry.spans.size(); ++spanIndex)
  {
    const int subdivisions =
        geometry.spans[spanIndex].interpolation == GeometryInterpolation::Bezier ? 16 : 1;
    for (int subdivision = 1; subdivision <= subdivisions; ++subdivision)
    {
      const double t = static_cast<double>(subdivision) / subdivisions;
      const PhysicalCoordinate coordinate =
          geometry.spans[spanIndex].interpolation == GeometryInterpolation::Bezier
              ? evaluateBezier(geometry, spanIndex, t)
              : add(multiply(geometry.nodes[spanIndex].coordinate, 1.0 - t),
                    multiply(geometry.nodes[spanIndex + 1].coordinate, t));
      samples.push_back({coordinate, spanIndex});
    }
  }
  return samples;
}

double routeGeometryLength(const PhysicalRouteGeometry& geometry)
{
  const std::vector<RouteGeometrySample> samples = sampleRouteGeometry(geometry);
  double length = 0.0;
  for (size_t index = 1; index < samples.size(); ++index)
  {
    length += distance(samples[index - 1].coordinate, samples[index].coordinate);
  }
  return length;
}

std::vector<double> routeGeometryAnchorDistances(const PhysicalRouteGeometry& geometry,
                                                 const std::vector<AbstractNodeId>& routeNodes)
{
  std::vector<double> distances(routeNodes.size(), 0.0);
  if (routeNodes.empty())
  {
    return distances;
  }

  const std::vector<RouteGeometrySample> samples = sampleRouteGeometry(geometry);
  double travelled = 0.0;
  size_t routeIndex = 0;
  for (size_t sampleIndex = 0; sampleIndex < samples.size() && routeIndex < routeNodes.size();
       ++sampleIndex)
  {
    if (sampleIndex > 0)
    {
      travelled += distance(samples[sampleIndex - 1].coordinate, samples[sampleIndex].coordinate);
    }
    const auto geometryNode =
        std::find_if(geometry.nodes.begin(), geometry.nodes.end(),
                     [&](const PhysicalRouteGeometryNode& node)
                     {
                       return node.anchorNode.has_value() &&
                              sameNode(*node.anchorNode, routeNodes[routeIndex]) &&
                              node.coordinate.x == samples[sampleIndex].coordinate.x &&
                              node.coordinate.y == samples[sampleIndex].coordinate.y &&
                              node.coordinate.z == samples[sampleIndex].coordinate.z;
                     });
    if (geometryNode != geometry.nodes.end())
    {
      distances[routeIndex++] = travelled;
    }
  }
  return distances;
}

std::optional<RouteGeometryPose> routeGeometryPoseAtDistance(const PhysicalRouteGeometry& geometry,
                                                             double targetDistance)
{
  const std::vector<RouteGeometrySample> samples = sampleRouteGeometry(geometry);
  if (samples.size() < 2)
  {
    return std::nullopt;
  }

  double remaining = std::max(0.0, targetDistance);
  for (size_t index = 1; index < samples.size(); ++index)
  {
    const PhysicalCoordinate delta =
        subtract(samples[index].coordinate, samples[index - 1].coordinate);
    const double length = std::hypot(delta.x, delta.y, delta.z);
    if (length <= 0.0)
    {
      continue;
    }
    if (remaining <= length || index + 1 == samples.size())
    {
      const double fraction = std::clamp(remaining / length, 0.0, 1.0);
      return RouteGeometryPose{.coordinate =
                                   add(samples[index - 1].coordinate, multiply(delta, fraction)),
                               .direction = multiply(delta, 1.0 / length),
                               .spanIndex = samples[index].spanIndex};
    }
    remaining -= length;
  }
  return std::nullopt;
}

double routeGeometrySpeedLimitAtDistance(const PhysicalRouteGeometry& geometry, double distance)
{
  const std::optional<RouteGeometryPose> pose = routeGeometryPoseAtDistance(geometry, distance);
  if (!pose.has_value() || pose->spanIndex >= geometry.spans.size())
  {
    return 0.0;
  }
  return geometry.spans[pose->spanIndex].maximumSpeedKph;
}
} // namespace network
