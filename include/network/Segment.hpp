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
        
        void setStart(Node& node);
        void setEnd(Node& node);

    private:
        Node startNode;
        Node endNode;
    };
}