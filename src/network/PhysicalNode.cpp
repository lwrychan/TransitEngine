#include "network/PhysicalNode.hpp"
#include "network/PhysicalCoordinate.hpp"

network::PhysicalNode::PhysicalNode()
{
    this->nodeId = AbstractNodeId();
    this->nodeType = network::NodeType::GENERIC_NODE;
    this->coordinate = PhysicalCoordinate();
}

network::PhysicalNode::PhysicalNode(AbstractNodeId nodeId, NodeType nodeType,
                                    PhysicalCoordinate nodeCoordinate)
{
    this->nodeId = nodeId;
    this->nodeType = nodeType;
    this->coordinate = nodeCoordinate;
    this->initialized = true;
}

bool network::PhysicalNode::hasCoordinate(const PhysicalCoordinate& nodeCoordinate) const
{
    return this->initialized && this->coordinate.x == nodeCoordinate.x &&
           this->coordinate.y == nodeCoordinate.y && this->coordinate.z == nodeCoordinate.z;
}