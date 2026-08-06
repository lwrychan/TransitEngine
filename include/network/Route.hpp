#pragma once

#include <vector>

#include "Segment.hpp"

namespace network {
    class Route {
    public:
        Route() = default;
        Route(std::vector<Segment>& segments);

        void addSegment(Segment& segment);
    private:
        std::vector<Segment> segments;
    };
}