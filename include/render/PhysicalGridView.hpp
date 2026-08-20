#pragma once

namespace network
{
    class PhysicalNetwork;
}

struct RenderingConfig;

namespace render
{
    // Editor-only metre grid. It is a render aid, not part of the network model.
    void drawPhysicalGridView(
        const network::PhysicalNetwork& physicalNetwork,
        const RenderingConfig& config);
}
