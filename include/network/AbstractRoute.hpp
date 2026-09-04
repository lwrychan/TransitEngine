#pragma once

#include <utility>
#include <vector>

#include "AbstractSegment.hpp"
#include "Identifiers.hpp"

namespace network
{
struct RouteColor
{
  float r = 0.25f;
  float g = 0.55f;
  float b = 0.95f;
};

// Ordered logical path through the topology. Geometry belongs to projections.
class AbstractRoute
{
  public:
  AbstractRoute() = default;
  explicit AbstractRoute(std::vector<AbstractSegmentId> segments) : segments(std::move(segments)) {}

  AbstractRoute(std::string name, std::vector<AbstractNodeId> nodes, RouteColor color = {})
      : name(std::move(name)), nodes(std::move(nodes)), color(color)
  {
  }

  const std::vector<AbstractSegmentId>& getSegments() const
  {
    return segments;
  }
  const std::string& getName() const
  {
    return name;
  }
  const std::vector<AbstractNodeId>& getNodes() const
  {
    return nodes;
  }
  const RouteColor& getColor() const
  {
    return color;
  }
  void addSegment(AbstractSegmentId segmentId)
  {
    segments.push_back(segmentId);
  }
  void addNode(AbstractNodeId nodeId)
  {
    nodes.push_back(nodeId);
  }
  void setName(std::string newName)
  {
    name = std::move(newName);
  }
  void setNodes(std::vector<AbstractNodeId> newNodes)
  {
    nodes = std::move(newNodes);
  }
  void setColor(RouteColor newColor)
  {
    color = newColor;
  }

  private:
  std::string name;
  std::vector<AbstractNodeId> nodes;
  std::vector<AbstractSegmentId> segments;
  RouteColor color;
};
} // namespace network
