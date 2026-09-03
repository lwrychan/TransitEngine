#pragma once

#include <vector>

#include "AbstractNode.hpp"
#include "AbstractRoute.hpp"
#include "AbstractSegment.hpp"

namespace network
{
// Canonical, geometry-free topology. All presentation layers derive from it.
class AbstractNetwork
{
public:
    const std::vector<AbstractNode>& getNodes() const
    {
        return nodes;
    }
    const std::vector<AbstractSegment>& getSegments() const
    {
        return segments;
    }
    const std::vector<AbstractRoute>& getRoutes() const
    {
        return routes;
    }

private:
    std::vector<AbstractNode> nodes;
    std::vector<AbstractSegment> segments;
    std::vector<AbstractRoute> routes;
};
} // namespace network
