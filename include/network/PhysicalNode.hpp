#pragma once
#include "Identifiers.hpp"
#include "NodeType.hpp"
#include "network/PhysicalCoordinate.hpp"
#include "network/PhysicalNode.hpp"

namespace network
{
class PhysicalNode
{
  public:
  PhysicalNode();
  PhysicalNode(AbstractNodeId nodeId, NodeType nodeType, PhysicalCoordinate nodeCoordinate);

  bool hasCoordinate(const PhysicalCoordinate& nodeCoordinate) const;
  AbstractNodeId getNodeId() const
  {
    return nodeId;
  }
  NodeType getNodeType() const
  {
    return nodeType;
  }
  void setType(NodeType newType)
  {
    nodeType = newType;
  }
  const PhysicalCoordinate& getCoordinate() const
  {
    return coordinate;
  }
  void setCoordinate(const PhysicalCoordinate& newCoordinate)
  {
    coordinate = newCoordinate;
  }

  private:
  AbstractNodeId nodeId;
  NodeType nodeType = NodeType::GENERIC_NODE;
  PhysicalCoordinate coordinate;
  bool initialized = false;
};
} // namespace network