#pragma once

#include "AbstractNode.hpp"
#include "Identifiers.hpp"

namespace network
{
// Logical connection; physical and map layers project this relationship differently.
class AbstractSegment
{
public:
    AbstractSegment() = default;
    AbstractSegment(AbstractNodeId start, AbstractNodeId end) : startNodeId(start), endNodeId(end)
    {
    }

    AbstractNodeId getStartNodeId() const
    {
        return startNodeId;
    }
    AbstractNodeId getEndNodeId() const
    {
        return endNodeId;
    }

private:
    AbstractNodeId startNodeId{-1, -1};
    AbstractNodeId endNodeId{-1, -1};
};
} // namespace network
