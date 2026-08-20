#pragma once

#include <utility>
#include <vector>

#include "AbstractNetwork.hpp"
#include "Identifiers.hpp"

namespace network
{
    struct PhysicalPoint
    {
        double xMeters = 0.0;
        double yMeters = 0.0;
    };

    // The real-world shape of one abstract segment, expressed in metres.
    struct PhysicalSegmentGeometry
    {
        AbstractSegmentId abstractSegmentId{-1, -1};
        std::vector<PhysicalPoint> path;
    };

    // Physical projection of AbstractNetwork: identical topology plus real track geometry.
    class PhysicalNetwork
    {
    public:
        explicit PhysicalNetwork(const AbstractNetwork& abstractNetwork)
            : abstractNetwork(&abstractNetwork) {}

        const AbstractNetwork& getAbstractNetwork() const { return *abstractNetwork; }
        const std::vector<PhysicalSegmentGeometry>& getSegmentGeometries() const {
            return segmentGeometries;
        }

        void setSegmentGeometry(AbstractSegmentId segmentId, std::vector<PhysicalPoint> path) {
            for (PhysicalSegmentGeometry& geometry : segmentGeometries) {
                if (geometry.abstractSegmentId.id == segmentId.id
                    && geometry.abstractSegmentId.generation == segmentId.generation) {
                    geometry.path = std::move(path);
                    return;
                }
            }
            segmentGeometries.push_back({segmentId, std::move(path)});
        }

    private:
        const AbstractNetwork* abstractNetwork;
        std::vector<PhysicalSegmentGeometry> segmentGeometries;
    };
}
