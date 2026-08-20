#pragma once

#include <algorithm>

namespace network
{
    // Simulation and editor projection. Coordinates are real grid cells.
    class PhysicalNetwork
    {
    public:
        PhysicalNetwork(int gridWidth = 32, int gridHeight = 22)
            : gridWidth(std::max(1, gridWidth)), gridHeight(std::max(1, gridHeight)) {}

        int getGridWidth() const { return gridWidth; }
        int getGridHeight() const { return gridHeight; }

        void setGridSize(int width, int height) {
            gridWidth = std::max(1, width);
            gridHeight = std::max(1, height);
        }

    private:
        int gridWidth;
        int gridHeight;
    };
}
