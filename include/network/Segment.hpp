#pragma once

#include "Node.hpp"

namespace network
{
    class Segment
    {
    public:
        Segment() = default;
        ~Segment() = default;

        void initialize();
        void update(double timestep);

    private:
        Node startNode;
        Node endNode;
    };
}