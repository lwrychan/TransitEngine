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
        void initialize();
        void update(double timestep);

        const std::vector<Node>& getNodes();
        const std::vector<Segment>& getSegments();
        const std::vector<Route>& getRoutes();
    private:
        std::vector<Node> nodes;
        std::vector<Segment> segments;
        std::vector<Route> routes;
    };
}