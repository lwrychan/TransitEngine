#include <vector>

#include "network/Node.hpp"
#include "network/Segment.hpp"
#include "network/Route.hpp"
#include "network/Network.hpp"

namespace network {
    const std::vector<Node>& Network::getNodes() {
        return this->nodes;
    }
    const std::vector<Segment>& Network::getSegments() {
        return this->segments;
    }
    const std::vector<Route>& Network::getRoutes() {
        return this->routes;
    }
}

