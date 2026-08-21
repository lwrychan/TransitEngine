#pragma once

namespace network
{
    class PhysicalNetwork;
}

struct RenderingConfig;

namespace render
{
    void drawPhysicalGridView(
        const network::PhysicalNetwork& physicalNetwork,
        const RenderingConfig& config);
}
