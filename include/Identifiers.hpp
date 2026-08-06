#pragma once

template<typename Identifier>
struct Id {
    int id;
    int generation;
};
using NodeId = Id<struct NodeIdentifier>;
using SegmentId = Id<struct SegmentIdentifier>;
using RouteId = Id<struct RouteIdentifier>;
using VehicleId = Id<struct VehicleIndentifier>;
