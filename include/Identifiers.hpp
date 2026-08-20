#pragma once

template<typename Identifier>
struct Id {
    int id;
    int generation;
};
using AbstractNodeId = Id<struct AbstractNodeIdentifier>;
using AbstractSegmentId = Id<struct AbstractSegmentIdentifier>;
using AbstractRouteId = Id<struct AbstractRouteIdentifier>;
using VehicleId = Id<struct VehicleIndentifier>;
