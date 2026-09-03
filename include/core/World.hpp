#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "CoreConfig.hpp"
#include "network/AbstractNetwork.hpp"
#include "network/MapNetwork.hpp"
#include "network/PhysicalNetwork.hpp"
#include "network/PhysicalCoordinate.hpp"
#include "Identifiers.hpp"
#include "network/NodeType.hpp"
#include "network/PhysicalNode.hpp"
#include "vehicle/Vehicle.hpp"

#include "SlotMap.hpp"

namespace core
{
    struct PhysicalVehiclePose
    {
        network::PhysicalCoordinate coordinate;
        network::PhysicalCoordinate direction{1.0, 0.0, 0.0};
    };

    class World
    {
    public:
        World(const CoreConfig& config);

        VehicleId addVehicle(vehicle::Vehicle vehicle);
        void removeVehicle(size_t vehicleIndex);
        void assignVehicleToRoute(VehicleId vehicle, AbstractRouteId route);
        void clearVehicleRoute(VehicleId vehicle);
        void setVehicleRouteStops(VehicleId vehicle, std::vector<vehicle::RouteStop> stops);
        std::optional<PhysicalVehiclePose> getVehiclePose(VehicleId vehicle) const;

        void addNode(
            network::NodeType nodeType,
            const network::PhysicalCoordinate& nodeCoordinate,
            const std::string& nodeName = "New Node");
        bool updateNode(
            AbstractNodeId node,
            network::NodeType nodeType,
            const network::PhysicalCoordinate& nodeCoordinate,
            const std::string& nodeName);
        AbstractRouteId addRoute(
            const std::string& routeName,
            const std::vector<AbstractNodeId>& nodeIds,
            const network::RouteColor& color = {});
        bool updateRoute(
            AbstractRouteId route,
            const std::string& routeName,
            const std::vector<AbstractNodeId>& nodeIds,
            const network::RouteColor& color);
        void removeNode(AbstractNodeId node);
        void removeRoute(AbstractRouteId route);
        bool hasNodeAt(const network::PhysicalCoordinate& nodeCoordinate) const;
        void setActiveNodeAt(const network::PhysicalCoordinate& nodeCoordinate);
        const std::optional<AbstractNodeId>& getActiveNode() const { return activeNode; }
        void setActiveRoute(AbstractRouteId route);
        const std::optional<AbstractRouteId>& getActiveRoute() const { return activeRoute; }
        network::AbstractNode& getNode(AbstractNodeId node);
        const network::AbstractNode& getNode(AbstractNodeId node) const;
        network::PhysicalNode& getPhysicalNode(AbstractNodeId node);
        const network::PhysicalNode& getPhysicalNode(AbstractNodeId node) const;
        network::AbstractRoute& getRoute(AbstractRouteId route);
        const network::AbstractRoute& getRoute(AbstractRouteId route) const;

        network::AbstractNetwork& getAbstractNetwork();
        const network::AbstractNetwork& getAbstractNetwork() const;
        network::PhysicalNetwork& getPhysicalNetwork();
        const network::PhysicalNetwork& getPhysicalNetwork() const;
        network::MapNetwork& getMapNetwork();
        const network::MapNetwork& getMapNetwork() const;

        void tick(double simulationDelta);

        uint64_t getTickCount() const;
        size_t getVehicleCount() const;
        size_t getAbstractNodeCount() const;
        size_t getAbstractSegmentCount() const;
        size_t getAbstractRouteCount() const;

        void forEachVehicle(const std::function<void(VehicleId, vehicle::Vehicle&)>& callback);

        void forEachNode(const std::function<void(AbstractNodeId, network::AbstractNode&)>& callback);
        void forEachRoute(const std::function<void(AbstractRouteId, network::AbstractRoute&)>& callback);

    private:
        CoreConfig globalConfig;
        network::AbstractNetwork abstractNetwork;
        network::PhysicalNetwork physicalNetwork;
        network::MapNetwork mapNetwork;
        SlotMap<vehicle::Vehicle, VehicleId> vehicles;

        SlotMap<network::AbstractNode, AbstractNodeId> nodes;
        SlotMap<network::AbstractSegment, AbstractSegmentId> segments;
        SlotMap<network::AbstractRoute, AbstractRouteId> routes;

        std::vector<network::PhysicalNode> physicalNodes;
        std::optional<AbstractNodeId> activeNode;
        std::optional<AbstractRouteId> activeRoute;

        uint64_t tickCount = 0;

        std::vector<double> getRouteNodeDistances(
            const std::vector<AbstractNodeId>& nodesOnRoute) const;
    };
}
