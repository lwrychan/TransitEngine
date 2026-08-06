#pragma once

#include <string>

namespace network
{
    class Node
    {
    public:
        Node() = default;
        ~Node() = default;
    private:
        std::string name;
    };
}