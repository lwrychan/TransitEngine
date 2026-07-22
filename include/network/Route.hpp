#pragma once

#include <vector>

#include "Segment.hpp"

namespace network {
    class Route {
    public:
        Route() = default;
        ~Route() = default;
    private:
        std::vector<Segment> segments;
    };
}