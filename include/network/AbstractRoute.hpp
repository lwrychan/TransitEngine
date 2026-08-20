#pragma once

#include <utility>
#include <vector>

#include "AbstractSegment.hpp"

namespace network
{
    // Ordered logical path through the topology. Geometry belongs to projections.
    class AbstractRoute
    {
    public:
        AbstractRoute() = default;
        explicit AbstractRoute(std::vector<AbstractSegmentId> segments)
            : segments(std::move(segments)) {}

        const std::vector<AbstractSegmentId>& getSegments() const { return segments; }
        void addSegment(AbstractSegmentId segmentId) { segments.push_back(segmentId); }

    private:
        std::vector<AbstractSegmentId> segments;
    };
}
