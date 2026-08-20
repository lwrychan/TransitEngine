#pragma once

#include <string>
#include <utility>

namespace network
{
    // Logical endpoint in the transit topology. It deliberately has no position.
    class AbstractNode
    {
    public:
        AbstractNode() = default;
        explicit AbstractNode(std::string name) : name(std::move(name)) {}

        const std::string& getName() const { return name; }

    private:
        std::string name;
    };
}
