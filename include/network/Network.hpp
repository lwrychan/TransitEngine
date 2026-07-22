#pragma once

#include <vector>

#include "Node.hpp"
#include "Segment.hpp"
#include "Route.hpp"

namespace network
{
    class Network
    {
    public:
        Network() = default;
        ~Network() = default;

        void initialize();
        void update(double timestep);

    private:
        std::vector<Node> nodes;
        std::vector<Segment> segments;
        std::vector<Route> routes;
    };
}