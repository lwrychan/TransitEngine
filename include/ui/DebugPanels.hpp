#pragma once

#include <string>
#include <vector>

#include "Identifiers.hpp"
#include "Simulation.hpp"
#include "network/AbstractRoute.hpp"
#include "network/PhysicalCoordinate.hpp"
#include "render/GridTool.hpp"

namespace ui
{
class LogBuffer
{
  public:
  static LogBuffer& instance();

  void add(const std::string& message);
  void clear();
  const std::vector<std::string>& getLines() const;

  private:
  static constexpr size_t MAX_LINES = 200;
  std::vector<std::string> lines;
};

struct VehicleCreationDraft
{
  char displayName[64] = "New Vehicle";
  float maxSpeedKph = 80.0f;
  float accelerationKphPerSecond = 15.0f;
  float decelerationKphPerSecond = 15.0f;
  int passengerCapacity = 100;
};

VehicleCreationDraft& getVehicleCreationDraft();
void requestVehicleCreation();

void requestNodeCreation(const network::PhysicalCoordinate& coordinate);
void requestNodeEdit(core::World& world, AbstractNodeId node);
void requestNodeEdit(core::World& world, AbstractNodeId node,
                     const network::PhysicalCoordinate& coordinate);
void requestNodeMove(core::World& world, AbstractNodeId node,
                     const network::PhysicalCoordinate& coordinate);
void toggleRouteNode(AbstractNodeId node);
bool isRouteNodeSelected(AbstractNodeId node);
void clearRouteNodeSelection();
const std::vector<AbstractNodeId>& getRouteSelectionNodes();
network::RouteColor getRouteDraftColor();
void requestRouteSave();
void requestRouteEdit(core::World& world, AbstractRouteId route);
bool isRouteModalOpen();
void drawRouteControls(core::World& world, render::Tool activeTool);
bool isModalOpen();
bool shouldShowVehicleSpeeds();
void drawEditorDialogs(Simulation& simulation);
void drawDeleteConfirmation(core::World& world);
void drawNodeMoveConfirmation(core::World& world);

void drawSimulationControls(Simulation& simulation, bool showDebugPanels);
void drawInspector(Simulation& simulation, bool showDebugPanels);
void drawPerformancePanel(Simulation& simulation);
void drawWorldSummary(Simulation& simulation);
void drawEditorPanel(Simulation& simulation, bool showDebugPanels);
void drawLogPanel();
} // namespace ui
