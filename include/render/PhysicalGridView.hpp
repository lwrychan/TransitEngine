#pragma once

namespace network
{
}

namespace core
{
    class World;
}

struct RenderingConfig;

namespace render
{
    enum class Tool
    {
        Pointer,
        Node,
        Route
    };

    void drawPhysicalGridView(
        core::World& world,
        const RenderingConfig& config,
        Tool activeTool,
        bool modalOpen);

    double zeroRound(double number);
}
