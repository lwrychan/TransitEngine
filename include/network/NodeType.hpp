#pragma once

namespace network
{
enum NodeType
{
    GENERIC_NODE,
    STATION_RAIL,
    STATION_BUS,
    INTERSECTION_ROAD,
    OBJECT_BUILDING,
    OBJECT_WATER,
    OBJECT_HILL,
    OBJECT_MOUNTAIN,
    MASK_ELEVATION,
};

inline const char* nodeTypeName(NodeType nodeType)
{
    switch (nodeType)
    {
    case GENERIC_NODE:
        return "Generic";
    case STATION_RAIL:
        return "Rail Station";
    case STATION_BUS:
        return "Bus Stop";
    case INTERSECTION_ROAD:
        return "Road Intersection";
    case OBJECT_BUILDING:
        return "Building";
    case OBJECT_WATER:
        return "Water";
    case OBJECT_HILL:
        return "Hill";
    case OBJECT_MOUNTAIN:
        return "Mountain";
    case MASK_ELEVATION:
        return "Elevation Mask";
    }
    return "Unknown";
}
} // namespace network
