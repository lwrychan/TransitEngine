#pragma once
#include "Identifiers.hpp"
#include "NodeType.hpp"
#include "network/PhysicalNode.hpp"
#include "network/PhysicalCoordinate.hpp"

namespace network {
    class PhysicalNode {
    public:
        PhysicalNode();
        PhysicalNode(
            AbstractNodeId nodeId,
            NodeType nodeType,
            PhysicalCoordinate nodeCoordinate);

        bool hasCoordinate(const PhysicalCoordinate& nodeCoordinate) const;
        AbstractNodeId getNodeId() const { return nodeId; }
        NodeType getNodeType() const { return nodeType; }
        void setType(NodeType newType) { nodeType = newType; }
        const PhysicalCoordinate& getCoordinate() const { return coordinate; }
        void setCoordinate(const PhysicalCoordinate& newCoordinate) { coordinate = newCoordinate; }

    private:
        AbstractNodeId nodeId;
        NodeType nodeType = NodeType::GENERIC_NODE;
        PhysicalCoordinate coordinate;
        bool initialized = false;
    };
}