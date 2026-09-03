#include "ui/DebugPanels.hpp"
#include "ui/UIStyle.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <imgui.h>
#include <sstream>
#include <string>

#include "core/World.hpp"
#include "vehicle/Vehicle.hpp"

namespace ui
{
namespace
{
    struct PanelLayout
    {
        ImVec2 simulationPosition;
        ImVec2 simulationSize;
        ImVec2 performancePosition;
        ImVec2 performanceSize;
        ImVec2 logPosition;
        ImVec2 logSize;
        ImVec2 vehiclePosition;
        ImVec2 vehicleSize;
        ImVec2 worldPosition;
        ImVec2 worldSize;
        ImVec2 abstractNetworkPosition;
        ImVec2 abstractNetworkSize;
    };

    PanelLayout getPanelLayout()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float margin = 10.0f;
        constexpr float toolbarOffset = 70.0f;
        const float panelGap = 10.0f;
        const float leftWidth = std::clamp(viewport->WorkSize.x * 0.24f, 260.0f, 320.0f);
        const float rightWidth = std::clamp(viewport->WorkSize.x * 0.24f, 260.0f, 320.0f);
        const float contentHeight =
            std::max(180.0f, viewport->WorkSize.y - margin * 2.0f - toolbarOffset);
        const float simulationHeight = std::min(190.0f, contentHeight * 0.30f);
        const float performanceHeight = std::min(275.0f, contentHeight * 0.42f);
        const float vehicleHeight = std::min(310.0f, contentHeight * 0.45f);
        const float worldHeight = std::min(190.0f, contentHeight * 0.28f);

        const float leftX = viewport->WorkPos.x + margin;
        const float topY = viewport->WorkPos.y + margin + toolbarOffset;
        const float rightX = viewport->WorkPos.x + viewport->WorkSize.x - margin - rightWidth;

        const float leftLogY = topY + simulationHeight + panelGap + performanceHeight + panelGap;
        const float rightAbstractY = topY + vehicleHeight + panelGap + worldHeight + panelGap;
        const float bottomY = viewport->WorkPos.y + viewport->WorkSize.y - margin;

        return {.simulationPosition = ImVec2(leftX, topY),
                .simulationSize = ImVec2(leftWidth, simulationHeight),
                .performancePosition = ImVec2(leftX, topY + simulationHeight + panelGap),
                .performanceSize = ImVec2(leftWidth, performanceHeight),
                .logPosition = ImVec2(leftX, leftLogY),
                .logSize = ImVec2(leftWidth, std::max(100.0f, bottomY - leftLogY)),
                .vehiclePosition = ImVec2(rightX, topY),
                .vehicleSize = ImVec2(rightWidth, vehicleHeight),
                .worldPosition = ImVec2(rightX, topY + vehicleHeight + panelGap),
                .worldSize = ImVec2(rightWidth, worldHeight),
                .abstractNetworkPosition = ImVec2(rightX, rightAbstractY),
                .abstractNetworkSize =
                    ImVec2(rightWidth, std::max(100.0f, bottomY - rightAbstractY))};
    }

    void applyPanelLayout(ImVec2 position, ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    }

    constexpr ImGuiWindowFlags ResponsivePanelFlags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse;

    bool whiteButton(const char* label, ImVec2 size = ImVec2(0.0f, 0.0f))
    {
        return ui::button(label, size);
    }

    bool whiteSmallButton(const char* label)
    {
        return ui::smallButton(label);
    }

    bool whiteTab(const char* label)
    {
        return ui::beginTabItem(label);
    }

} // namespace

LogBuffer& LogBuffer::instance()
{
    static LogBuffer buffer;
    return buffer;
}

void LogBuffer::add(const std::string& message)
{
    this->lines.push_back(message);
    if (this->lines.size() > MAX_LINES)
    {
        this->lines.erase(this->lines.begin());
    }
}

void LogBuffer::clear()
{
    this->lines.clear();
}

const std::vector<std::string>& LogBuffer::getLines() const
{
    return this->lines;
}

VehicleCreationDraft& getVehicleCreationDraft()
{
    static VehicleCreationDraft draft;
    return draft;
}

void loadVehicleCreationPreset(const char* displayName, float maxSpeedKph,
                               float accelerationKphPerSecond, float decelerationKphPerSecond,
                               int passengerCapacity)
{
    VehicleCreationDraft& draft = getVehicleCreationDraft();
    std::strncpy(draft.displayName, displayName, sizeof(draft.displayName) - 1);
    draft.displayName[sizeof(draft.displayName) - 1] = '\0';
    draft.maxSpeedKph = maxSpeedKph;
    draft.accelerationKphPerSecond = accelerationKphPerSecond;
    draft.decelerationKphPerSecond = decelerationKphPerSecond;
    draft.passengerCapacity = passengerCapacity;
}

namespace
{
    enum class DeleteConfirmationTarget
    {
        None,
        Node,
        Route
    };

    bool showVehicleCreationModal = false;
    bool showVehicleSpeeds = false;
    bool showNodeCreationModal = false;
    bool showNodeMoveConfirmation = false;
    DeleteConfirmationTarget deleteConfirmationTarget = DeleteConfirmationTarget::None;
    std::optional<AbstractNodeId> editingNode;
    std::optional<AbstractRouteId> routePendingDeletion;
    std::optional<AbstractNodeId> movingNode;
    network::PhysicalCoordinate moveOrigin{};
    network::PhysicalCoordinate moveDestination{};
    std::vector<AbstractNodeId> routeNodes;
    bool showRouteSaveModal = false;
    std::optional<AbstractRouteId> editingRoute;
    char routeName[64] = "New Route";
    network::RouteColor routeColor{};
    struct RouteVehicleAssignmentDraft
    {
        VehicleId vehicle;
        std::vector<vehicle::RouteStop> stops;
    };
    std::vector<RouteVehicleAssignmentDraft> routeVehicles;

    bool sameNode(AbstractNodeId left, AbstractNodeId right)
    {
        return left.id == right.id && left.generation == right.generation;
    }

    bool sameVehicle(VehicleId left, VehicleId right)
    {
        return left.id == right.id && left.generation == right.generation;
    }

    bool isRouteVehicleSelected(VehicleId vehicle)
    {
        return std::find_if(routeVehicles.begin(), routeVehicles.end(),
                            [vehicle](const RouteVehicleAssignmentDraft& selected)
                            { return sameVehicle(selected.vehicle, vehicle); }) !=
               routeVehicles.end();
    }

    RouteVehicleAssignmentDraft* getRouteVehicleDraft(VehicleId vehicle)
    {
        const auto found = std::find_if(routeVehicles.begin(), routeVehicles.end(),
                                        [vehicle](const RouteVehicleAssignmentDraft& selected)
                                        { return sameVehicle(selected.vehicle, vehicle); });
        return found == routeVehicles.end() ? nullptr : &*found;
    }

    bool isStopSelected(const std::vector<vehicle::RouteStop>& stops, AbstractNodeId node)
    {
        return std::find_if(stops.begin(), stops.end(), [node](const vehicle::RouteStop& selected)
                            { return sameNode(selected.node, node); }) != stops.end();
    }

    std::vector<vehicle::RouteStop> localStopTemplate()
    {
        std::vector<vehicle::RouteStop> stops;
        for (AbstractNodeId node : routeNodes)
        {
            if (!isStopSelected(stops, node))
            {
                stops.push_back({node, 20.0});
            }
        }
        return stops;
    }

    std::vector<vehicle::RouteStop> expressStopTemplate()
    {
        if (routeNodes.empty())
        {
            return {};
        }
        if (routeNodes.size() == 1 || sameNode(routeNodes.front(), routeNodes.back()))
        {
            return {{routeNodes.front(), 20.0}};
        }
        return {{routeNodes.front(), 20.0}, {routeNodes.back(), 20.0}};
    }

    std::vector<vehicle::RouteStop> validStopsForRoute(const std::vector<vehicle::RouteStop>& stops)
    {
        std::vector<vehicle::RouteStop> validStops;
        for (AbstractNodeId node : routeNodes)
        {
            const auto stop =
                std::find_if(stops.begin(), stops.end(), [node](const vehicle::RouteStop& candidate)
                             { return sameNode(candidate.node, node); });
            if (stop != stops.end() && !isStopSelected(validStops, node))
            {
                validStops.push_back(*stop);
            }
        }
        return validStops;
    }

    void requestNodeDeletion()
    {
        deleteConfirmationTarget = DeleteConfirmationTarget::Node;
    }

    void requestRouteDeletion(AbstractRouteId route)
    {
        routePendingDeletion = route;
        deleteConfirmationTarget = DeleteConfirmationTarget::Route;
    }
    network::PhysicalCoordinate nodeCreationCoordinate{};
    network::NodeType nodeCreationType = network::NodeType::GENERIC_NODE;
    char nodeCreationName[64] = "New Node";
    char nodeCreationCoordinateInputs[3][32]{};
    bool coordinateInputActive[3]{};

    void formatCoordinateInput(char* buffer, double coordinate)
    {
        const double displayCoordinate = coordinate == 0.0 ? 0.0 : coordinate;
        std::snprintf(buffer, 32, "%.0f m", displayCoordinate);
    }

    bool parseCoordinateInput(const char* input, double& meters, std::string& errorReason)
    {
        char* end = nullptr;
        const double value = std::strtod(input, &end);
        if (end == input || !std::isfinite(value))
        {
            errorReason = "Enter a number followed by m or km.";
            return false;
        }

        while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)))
        {
            ++end;
        }

        std::string unit;
        while (*end != '\0' && std::isalpha(static_cast<unsigned char>(*end)))
        {
            unit.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(*end))));
            ++end;
        }

        while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)))
        {
            ++end;
        }
        if (*end != '\0')
        {
            errorReason = "Unexpected characters after the unit.";
            return false;
        }

        double multiplier = 1.0;
        if (unit == "km")
        {
            multiplier = 1000.0;
        }
        else if (unit.empty() || unit == "m")
        {
            if (std::round(value) != value)
            {
                errorReason = "Meter values must be whole numbers.";
                return false;
            }
        }
        else
        {
            errorReason = "Only m and km units are supported.";
            return false;
        }

        meters = value * multiplier;
        if (!std::isfinite(meters))
        {
            errorReason = "The value is out of range.";
            return false;
        }

        meters = std::round(meters);
        return true;
    }

    void setCoordinateInputs(const network::PhysicalCoordinate& coordinate)
    {
        formatCoordinateInput(nodeCreationCoordinateInputs[0], coordinate.x);
        formatCoordinateInput(nodeCreationCoordinateInputs[1], coordinate.y);
        formatCoordinateInput(nodeCreationCoordinateInputs[2], coordinate.z);
    }

    bool drawCoordinateInput(const char* label, char* buffer, double& coordinate)
    {
        const int inputIndex = label[0] == 'X' ? 0 : (label[0] == 'Y' ? 1 : 2);
        double parsedValue = coordinate;
        std::string errorReason;
        const bool isValidBeforeInput = parseCoordinateInput(buffer, parsedValue, errorReason);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, coordinateInputActive[inputIndex]
                                                    ? ImVec4(0.68f, 0.68f, 0.68f, 1.0f)
                                                    : ImVec4(0.78f, 0.78f, 0.78f, 1.0f));
        if (!isValidBeforeInput)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.12f, 0.12f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }

        const bool wasSubmitted =
            ImGui::InputText(label, buffer, 32, ImGuiInputTextFlags_EnterReturnsTrue);

        coordinateInputActive[inputIndex] = ImGui::IsItemActive();
        const bool shouldCommit = wasSubmitted || ImGui::IsItemDeactivatedAfterEdit();
        const bool isValid = parseCoordinateInput(buffer, parsedValue, errorReason);

        if (shouldCommit && isValid)
        {
            coordinate = parsedValue;
            formatCoordinateInput(buffer, coordinate);
        }

        if (!isValidBeforeInput)
        {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        ImGui::PopStyleColor();

        if (!isValid)
        {
            ImGui::TextColored(ImVec4(0.85f, 0.12f, 0.12f, 1.0f), "%s", errorReason.c_str());
        }

        return isValid;
    }

    void openVehicleCreationModal()
    {
        showVehicleCreationModal = true;
    }

    bool createVehicleFromDraft(core::World& world, VehicleCreationDraft& draft)
    {
        if (draft.displayName[0] == '\0')
        {
            LogBuffer::instance().add("Vehicle creation failed: display name is required.");
            return false;
        }

        if (draft.maxSpeedKph <= 0.0f || draft.accelerationKphPerSecond <= 0.0f ||
            draft.decelerationKphPerSecond <= 0.0f || draft.passengerCapacity <= 0)
        {
            LogBuffer::instance().add(
                "Vehicle creation failed: all numeric properties must be positive.");
            return false;
        }

        world.addVehicle(vehicle::Vehicle(draft.displayName, draft.maxSpeedKph,
                                          draft.accelerationKphPerSecond,
                                          draft.decelerationKphPerSecond, draft.passengerCapacity));

        LogBuffer::instance().add(std::string("Created vehicle \"") + draft.displayName + "\".");
        return true;
    }

    void drawVehicleCreationModal(core::World& world)
    {
        if (showVehicleCreationModal)
        {
            ImGui::OpenPopup("Create Vehicle");
        }

        ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Create Vehicle", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            VehicleCreationDraft& draft = getVehicleCreationDraft();

            pushTextInputStyle();
            ImGui::InputText("Display Name", draft.displayName, sizeof(draft.displayName));
            ImGui::InputFloat("Max Speed (km/h)", &draft.maxSpeedKph, 1.0f, 10.0f, "%.1f");
            ImGui::InputFloat("Acceleration (km/h/s)", &draft.accelerationKphPerSecond, 1.0f, 5.0f,
                              "%.1f");
            ImGui::InputFloat("Deceleration (km/h/s)", &draft.decelerationKphPerSecond, 1.0f, 5.0f,
                              "%.1f");
            ImGui::InputInt("Passenger Capacity", &draft.passengerCapacity, 1, 10);
            popTextInputStyle();

            ImGui::Separator();
            ImGui::TextDisabled("Presets");

            if (whiteButton("Car"))
            {
                loadVehicleCreationPreset("Car", 100.0f, 10.0f, 10.0f, 4);
            }
            ImGui::SameLine();
            if (whiteButton("Truck"))
            {
                loadVehicleCreationPreset("Truck", 90.0f, 8.0f, 8.0f, 2);
            }
            ImGui::SameLine();
            if (whiteButton("Light Rail"))
            {
                loadVehicleCreationPreset("Light Rail", 60.0f, 12.0f, 12.0f, 150);
            }
            ImGui::SameLine();
            if (whiteButton("Subway"))
            {
                loadVehicleCreationPreset("Subway", 70.0f, 20.0f, 20.0f, 200);
            }
            ImGui::SameLine();
            if (whiteButton("Regional Train"))
            {
                loadVehicleCreationPreset("Regional Train", 120.0f, 20.0f, 20.0f, 300);
            }
            ImGui::SameLine();
            if (whiteButton("HSR"))
            {
                loadVehicleCreationPreset("HSR", 300.0f, 15.0f, 15.0f, 500);
            }

            ImGui::Separator();

            if (whiteButton("Confirm", ImVec2(120, 0)))
            {
                if (createVehicleFromDraft(world, draft))
                {
                    showVehicleCreationModal = false;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();

            if (whiteButton("Cancel", ImVec2(120, 0)))
            {
                showVehicleCreationModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
} // namespace

void requestVehicleCreation()
{
    openVehicleCreationModal();
}

void requestNodeCreation(const network::PhysicalCoordinate& coordinate)
{
    editingNode.reset();
    nodeCreationCoordinate = coordinate;
    nodeCreationType = network::NodeType::GENERIC_NODE;
    std::strncpy(nodeCreationName, "New Node", sizeof(nodeCreationName) - 1);
    nodeCreationName[sizeof(nodeCreationName) - 1] = '\0';
    setCoordinateInputs(nodeCreationCoordinate);
    showNodeCreationModal = true;
}

void requestNodeEdit(core::World& world, AbstractNodeId node)
{
    requestNodeEdit(world, node, world.getPhysicalNode(node).getCoordinate());
}

void requestNodeEdit(core::World& world, AbstractNodeId node,
                     const network::PhysicalCoordinate& coordinate)
{
    editingNode = node;
    const network::AbstractNode& abstractNode = world.getNode(node);
    const network::PhysicalNode& physicalNode = world.getPhysicalNode(node);
    std::strncpy(nodeCreationName, abstractNode.getName().c_str(), sizeof(nodeCreationName) - 1);
    nodeCreationName[sizeof(nodeCreationName) - 1] = '\0';
    nodeCreationType = physicalNode.getNodeType();
    nodeCreationCoordinate = coordinate;
    setCoordinateInputs(nodeCreationCoordinate);
    showNodeCreationModal = true;
}

bool isModalOpen()
{
    return showNodeCreationModal || showVehicleCreationModal || showNodeMoveConfirmation ||
           deleteConfirmationTarget != DeleteConfirmationTarget::None;
}

bool shouldShowVehicleSpeeds()
{
    return showVehicleSpeeds;
}

void toggleRouteNode(AbstractNodeId node)
{
    const auto existing =
        std::find_if(routeNodes.begin(), routeNodes.end(),
                     [node](AbstractNodeId selected) { return sameNode(selected, node); });
    if (existing == routeNodes.end())
    {
        routeNodes.push_back(node);
    }
    else if (routeNodes.size() >= 2 && sameNode(routeNodes.front(), node))
    {
        routeNodes.push_back(node);
    }
    else
    {
        routeNodes.erase(existing);
    }
}

bool isRouteNodeSelected(AbstractNodeId node)
{
    return std::find_if(routeNodes.begin(), routeNodes.end(), [node](AbstractNodeId selected)
                        { return sameNode(selected, node); }) != routeNodes.end();
}

void requestRouteSave()
{
    if (routeNodes.size() >= 2)
    {
        editingRoute.reset();
        routeVehicles.clear();
        showRouteSaveModal = true;
    }
}

void requestRouteEdit(core::World& world, AbstractRouteId route)
{
    const network::AbstractRoute& existingRoute = world.getRoute(route);
    editingRoute = route;
    routeNodes = existingRoute.getNodes();
    routeColor = existingRoute.getColor();
    routeVehicles.clear();
    world.forEachVehicle(
        [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
        {
            const std::optional<AbstractRouteId>& assignedRoute = vehicle.getAssignedRoute();
            if (assignedRoute.has_value() && assignedRoute->id == route.id &&
                assignedRoute->generation == route.generation)
            {
                routeVehicles.push_back({vehicleId, vehicle.getStops()});
            }
        });
    std::strncpy(routeName, existingRoute.getName().c_str(), sizeof(routeName) - 1);
    routeName[sizeof(routeName) - 1] = '\0';
    showRouteSaveModal = true;
}

bool isRouteModalOpen()
{
    return showRouteSaveModal;
}

void drawRouteControls(core::World& world)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = 42.0f;
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height),
        ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height), ImGuiCond_Always);
    ImGui::Begin("Route Selection", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Selected nodes: %zu", routeNodes.size());
    ImGui::SameLine();
    if (routeNodes.size() >= 2)
    {
        if (whiteButton("Save Route"))
        {
            requestRouteSave();
        }
    }
    else
    {
        ImGui::BeginDisabled();
        whiteButton("Save Route");
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (whiteButton("Clear"))
        routeNodes.clear();
    ImGui::End();

    if (showRouteSaveModal)
        ImGui::OpenPopup("Save Route");
    if (ImGui::BeginPopupModal("Save Route", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(editingRoute.has_value() ? "Edit Route" : "Create Route");
        pushTextInputStyle();
        ImGui::InputText("Route Name", routeName, sizeof(routeName));
        popTextInputStyle();
        float color[3] = {routeColor.r, routeColor.g, routeColor.b};
        if (ImGui::ColorEdit3("Route Color", color,
                              ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB))
        {
            routeColor = {color[0], color[1], color[2]};
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Vehicles on this route");
        bool hasVehicles = false;
        world.forEachVehicle(
            [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
            {
                hasVehicles = true;
                bool selected = isRouteVehicleSelected(vehicleId);
                ImGui::PushID(vehicleId.id * 1000 + vehicleId.generation);
                if (ImGui::Checkbox(vehicle.getDisplayName().c_str(), &selected))
                {
                    if (selected)
                    {
                        routeVehicles.push_back({vehicleId, localStopTemplate()});
                    }
                    else
                    {
                        routeVehicles.erase(
                            std::remove_if(
                                routeVehicles.begin(), routeVehicles.end(),
                                [vehicleId](const RouteVehicleAssignmentDraft& selectedVehicle)
                                { return sameVehicle(selectedVehicle.vehicle, vehicleId); }),
                            routeVehicles.end());
                    }
                }
                if (selected)
                {
                    RouteVehicleAssignmentDraft* assignment = getRouteVehicleDraft(vehicleId);
                    ImGui::Indent();
                    if (whiteSmallButton("Local (all nodes)"))
                    {
                        assignment->stops = localStopTemplate();
                    }
                    ImGui::SameLine();
                    if (whiteSmallButton("Express (endpoints)"))
                    {
                        assignment->stops = expressStopTemplate();
                    }
                    for (size_t nodeIndex = 0; nodeIndex < routeNodes.size(); ++nodeIndex)
                    {
                        const AbstractNodeId nodeId = routeNodes[nodeIndex];
                        bool stopsHere = isStopSelected(assignment->stops, nodeId);
                        const network::AbstractNode& node = world.getNode(nodeId);
                        ImGui::PushID(static_cast<int>(nodeIndex));
                        if (ImGui::Checkbox(node.getName().c_str(), &stopsHere))
                        {
                            if (stopsHere)
                            {
                                assignment->stops.push_back({nodeId, 20.0});
                            }
                            else
                            {
                                assignment->stops.erase(
                                    std::remove_if(assignment->stops.begin(),
                                                   assignment->stops.end(),
                                                   [nodeId](const vehicle::RouteStop& selectedStop)
                                                   { return sameNode(selectedStop.node, nodeId); }),
                                    assignment->stops.end());
                            }
                        }
                        if (stopsHere)
                        {
                            auto stop =
                                std::find_if(assignment->stops.begin(), assignment->stops.end(),
                                             [nodeId](const vehicle::RouteStop& selectedStop)
                                             { return sameNode(selectedStop.node, nodeId); });
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(95.0f);
                            pushTextInputStyle();
                            ImGui::InputDouble("Dwell (s)", &stop->dwellSeconds, 1.0, 5.0, "%.0f");
                            popTextInputStyle();
                            stop->dwellSeconds = std::max(0.0, stop->dwellSeconds);
                        }
                        ImGui::PopID();
                    }
                    ImGui::Unindent();
                }
                ImGui::PopID();
            });
        if (!hasVehicles)
        {
            ImGui::TextDisabled("Create a vehicle first, then assign it here.");
        }
        ImGui::Separator();
        ImGui::Text("Nodes in order");
        for (size_t index = 0; index < routeNodes.size(); ++index)
        {
            const AbstractNodeId nodeId = routeNodes[index];
            const network::AbstractNode& node = world.getNode(nodeId);
            const network::PhysicalCoordinate& coordinate =
                world.getPhysicalNode(nodeId).getCoordinate();
            ImGui::PushID(static_cast<int>(index));
            ImGui::Text("%zu. %s (%.0f, %.0f, %.0f m)", index + 1, node.getName().c_str(),
                        coordinate.x, coordinate.y, coordinate.z);
            ImGui::SameLine();
            if (index > 0 && whiteSmallButton("Up"))
                std::swap(routeNodes[index], routeNodes[index - 1]);
            ImGui::SameLine();
            if (index + 1 < routeNodes.size() && whiteSmallButton("Down"))
                std::swap(routeNodes[index], routeNodes[index + 1]);
            ImGui::SameLine();
            if (whiteSmallButton("Remove"))
                routeNodes.erase(routeNodes.begin() + index);
            ImGui::PopID();
        }
        if (whiteButton("Save", ImVec2(120, 0)))
        {
            if (routeName[0] != '\0' && routeNodes.size() >= 2)
            {
                AbstractRouteId savedRoute;
                if (editingRoute.has_value())
                {
                    world.updateRoute(*editingRoute, routeName, routeNodes, routeColor);
                    savedRoute = *editingRoute;
                }
                else
                {
                    savedRoute = world.addRoute(routeName, routeNodes, routeColor);
                }
                world.forEachVehicle(
                    [&](VehicleId vehicleId, vehicle::Vehicle& vehicle)
                    {
                        const bool selectedForRoute = isRouteVehicleSelected(vehicleId);
                        const std::optional<AbstractRouteId>& assignedRoute =
                            vehicle.getAssignedRoute();
                        const bool assignedToSavedRoute =
                            assignedRoute.has_value() && assignedRoute->id == savedRoute.id &&
                            assignedRoute->generation == savedRoute.generation;
                        if (selectedForRoute)
                        {
                            RouteVehicleAssignmentDraft* assignment =
                                getRouteVehicleDraft(vehicleId);
                            if (!assignedToSavedRoute)
                            {
                                world.assignVehicleToRoute(vehicleId, savedRoute);
                            }
                            world.setVehicleRouteStops(vehicleId,
                                                       validStopsForRoute(assignment->stops));
                        }
                        else if (!selectedForRoute && assignedToSavedRoute)
                        {
                            world.clearVehicleRoute(vehicleId);
                        }
                    });
                routeNodes.clear();
                routeVehicles.clear();
                editingRoute.reset();
                showRouteSaveModal = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (editingRoute.has_value() && whiteButton("Delete", ImVec2(120, 0)))
        {
            const AbstractRouteId route = *editingRoute;
            showRouteSaveModal = false;
            editingRoute.reset();
            requestRouteDeletion(route);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (whiteButton("Cancel", ImVec2(120, 0)))
        {
            editingRoute.reset();
            routeVehicles.clear();
            showRouteSaveModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void requestNodeMove(core::World& world, AbstractNodeId node,
                     const network::PhysicalCoordinate& coordinate)
{
    movingNode = node;
    moveOrigin = world.getPhysicalNode(node).getCoordinate();
    moveDestination = coordinate;
    showNodeMoveConfirmation = true;
}

void drawNodeMoveConfirmation(core::World& world)
{
    if (showNodeMoveConfirmation)
    {
        ImGui::OpenPopup("Confirm Node Move");
    }

    ImGui::SetNextWindowSize(ImVec2(380.0f, 0.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Confirm Node Move", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Move this node to the new location?");
        ImGui::Text("From: (%.0f, %.0f, %.0f) m", moveOrigin.x, moveOrigin.y, moveOrigin.z);
        ImGui::Text("To:   (%.0f, %.0f, %.0f) m", moveDestination.x, moveDestination.y,
                    moveDestination.z);

        ImGui::Separator();

        if (whiteButton("Move", ImVec2(120.0f, 0.0f)))
        {
            if (movingNode.has_value())
            {
                const AbstractNodeId node = *movingNode;
                const network::AbstractNode& abstractNode = world.getNode(node);
                const network::PhysicalNode& physicalNode = world.getPhysicalNode(node);
                if (world.updateNode(node, physicalNode.getNodeType(), moveDestination,
                                     abstractNode.getName()))
                {
                    LogBuffer::instance().add("Node Editor: Moved node.");
                }
                else
                {
                    LogBuffer::instance().add(
                        "Node Editor: A node already exists at that coordinate.");
                }
            }
            showNodeMoveConfirmation = false;
            movingNode.reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (whiteButton("Cancel", ImVec2(120.0f, 0.0f)))
        {
            showNodeMoveConfirmation = false;
            movingNode.reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void drawNodeCreationModal(core::World& world)
{
    if (showNodeCreationModal)
    {
        ImGui::OpenPopup("Node Editor");
    }

    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Node Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {

        pushTextInputStyle();
        ImGui::InputText("Name", nodeCreationName, sizeof(nodeCreationName));
        popTextInputStyle();
        const bool xValid =
            drawCoordinateInput("X", nodeCreationCoordinateInputs[0], nodeCreationCoordinate.x);
        const bool yValid =
            drawCoordinateInput("Y", nodeCreationCoordinateInputs[1], nodeCreationCoordinate.y);
        const bool zValid =
            drawCoordinateInput("Z", nodeCreationCoordinateInputs[2], nodeCreationCoordinate.z);

        ImGui::Separator();

        if (whiteButton("Rail Station"))
        {
            nodeCreationType = network::NodeType::STATION_RAIL;
            ui::LogBuffer::instance().add("Node Creator: Selected STATION_RAIL.");
        }
        ImGui::SameLine();
        if (whiteButton("Bus Stop"))
        {
            nodeCreationType = network::NodeType::STATION_BUS;
            ui::LogBuffer::instance().add("Node Creator: Selected STATION_BUS.");
        }

        ImGui::Separator();

        if (!xValid || !yValid || !zValid)
        {
            ImGui::BeginDisabled();
        }
        if (whiteButton("Confirm", ImVec2(120, 0)))
        {
            const bool wasEditing = editingNode.has_value();
            const bool updated = wasEditing
                                     ? world.updateNode(*editingNode, nodeCreationType,
                                                        nodeCreationCoordinate, nodeCreationName)
                                     : !world.hasNodeAt(nodeCreationCoordinate);
            if (updated)
            {
                if (!wasEditing)
                {
                    world.addNode(nodeCreationType, nodeCreationCoordinate, nodeCreationName);
                }
                showNodeCreationModal = false;
                editingNode.reset();
                ImGui::CloseCurrentPopup();
                ui::LogBuffer::instance().add(wasEditing ? "Node Editor: Updated node."
                                                         : "Node Creator: Created new Node.");
            }
            else
            {
                showNodeCreationModal = false;
                ImGui::CloseCurrentPopup();
                editingNode.reset();
                ui::LogBuffer::instance().add(
                    "Node Creator: A node already exists at this coordinate.");
            }
        }
        if (!xValid || !yValid || !zValid)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (editingNode.has_value() && whiteButton("Delete"))
        {
            showNodeCreationModal = false;
            requestNodeDeletion();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (whiteButton("Cancel", ImVec2(120, 0)))
        {
            showNodeCreationModal = false;
            editingNode.reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void drawDeleteConfirmation(core::World& world)
{
    if (deleteConfirmationTarget == DeleteConfirmationTarget::None)
    {
        return;
    }

    const bool deletingNode = deleteConfirmationTarget == DeleteConfirmationTarget::Node;
    const char* title = deletingNode ? "Delete Node" : "Delete Route";
    const bool itemExists =
        deletingNode ? editingNode.has_value() : routePendingDeletion.has_value();
    ImGui::OpenPopup(title);
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete this %s? This cannot be undone.", deletingNode ? "node" : "route");
        if (itemExists)
        {
            if (deletingNode)
            {
                const network::AbstractNode& node = world.getNode(*editingNode);
                ImGui::Text("Node: %s", node.getName().c_str());
            }
            else
            {
                const network::AbstractRoute& route = world.getRoute(*routePendingDeletion);
                ImGui::Text("Route: %s", route.getName().c_str());
            }
        }

        ImGui::Separator();
        if (whiteButton("Delete", ImVec2(120, 0)))
        {
            if (itemExists && deletingNode)
            {
                const AbstractNodeId deletedNode = *editingNode;
                world.removeNode(deletedNode);
                routeNodes.erase(std::remove_if(routeNodes.begin(), routeNodes.end(),
                                                [deletedNode](AbstractNodeId selected)
                                                { return sameNode(selected, deletedNode); }),
                                 routeNodes.end());
                LogBuffer::instance().add("Node Editor: Deleted node.");
            }
            else if (itemExists)
            {
                world.removeRoute(*routePendingDeletion);
                routeVehicles.clear();
                LogBuffer::instance().add(
                    "Route Editor: Deleted route and unassigned its vehicles.");
            }
            editingNode.reset();
            routePendingDeletion.reset();
            deleteConfirmationTarget = DeleteConfirmationTarget::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (whiteButton("Cancel", ImVec2(120, 0)))
        {
            if (deletingNode)
            {
                showNodeCreationModal = true;
            }
            routePendingDeletion.reset();
            deleteConfirmationTarget = DeleteConfirmationTarget::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void drawSimulationControls(Simulation& simulation)
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.simulationPosition, layout.simulationSize);
    ImGui::Begin("Simulation Controls", nullptr, ResponsivePanelFlags);

    const bool running = simulation.isRunning();
    if (running)
    {
        if (whiteButton("Pause"))
        {
            simulation.setRunning(false);
            LogBuffer::instance().add("Simulation paused.");
        }
    }
    else
    {
        if (whiteButton("Play"))
        {
            simulation.setRunning(true);
            LogBuffer::instance().add("Simulation started.");
        }
    }

    ImGui::SameLine();
    if (whiteButton("Step Once"))
    {
        simulation.requestStepOnce();
        LogBuffer::instance().add("Single simulation step requested.");
    }

    ImGui::Separator();

    ImGui::Text("Speed Multiplier");
    float speed = static_cast<float>(simulation.getSpeedMultiplier());
    if (ImGui::SliderFloat("##SpeedMultiplier", &speed, 0.25f, 4.0f, "%.2fx"))
    {
        simulation.setSpeedMultiplier(speed);
    }

    ImGui::SameLine();
    if (whiteButton("0.25x"))
    {
        simulation.setSpeedMultiplier(0.25);
    }
    ImGui::SameLine();
    if (whiteButton("1x"))
    {
        simulation.setSpeedMultiplier(1.0);
    }
    ImGui::SameLine();
    if (whiteButton("2x"))
    {
        simulation.setSpeedMultiplier(2.0);
    }
    ImGui::SameLine();
    if (whiteButton("4x"))
    {
        simulation.setSpeedMultiplier(4.0);
    }

    ImGui::Separator();

    core::World& world = simulation.getWorld();
    ImGui::Text("Simulation Time: %.2f s", simulation.getSimTime());
    ImGui::Text("Tick Count: %llu", static_cast<unsigned long long>(world.getTickCount()));
    ImGui::Text("Status: %s",
                running ? "Running" : (world.getTickCount() > 0 ? "Paused" : "Stopped"));

    ImGui::End();
}

void drawVehicleTab(Simulation& simulation)
{
    core::World& world = simulation.getWorld();

    if (whiteButton("Create Vehicle"))
    {
        openVehicleCreationModal();
    }

    ImGui::Separator();

    if (ImGui::BeginTable("Vehicles", 7,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollX))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Max Speed (km/h)");
        ImGui::TableSetupColumn("Accel (km/h/s)");
        ImGui::TableSetupColumn("Decel (km/h/s)");
        ImGui::TableSetupColumn("Capacity");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        world.forEachVehicle(
            [&](VehicleId id, vehicle::Vehicle& vehicle)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(vehicle.getDisplayName().c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d:%d", id.id, id.generation);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f", vehicle.getMaxOperatingSpeedKph());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f", vehicle.getAccelerationKphPerSecond());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.1f", vehicle.getDecelerationKphPerSecond());

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", vehicle.getPassengerCapacity());

                ImGui::TableSetColumnIndex(6);
                const size_t vehicleIndex = static_cast<size_t>(id.id);
                std::ostringstream removeLabel;
                removeLabel << "Remove##" << id.id << "_" << id.generation;
                if (whiteSmallButton(removeLabel.str().c_str()))
                {
                    world.removeVehicle(vehicleIndex);
                    LogBuffer::instance().add("Removed vehicle \"" + vehicle.getDisplayName() +
                                              "\".");
                }
            });

        ImGui::EndTable();
    }

    ImGui::Text("Total vehicles: %zu", world.getVehicleCount());
}

void drawPerformancePanel(Simulation& simulation)
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.performancePosition, layout.performanceSize);
    ImGui::Begin("Performance", nullptr, ResponsivePanelFlags);

    const double renderLatencyMs = simulation.getLastRenderLatencyMs();
    const double simulationLatencyMs = simulation.getLastSimulationLatencyMs();
    const double overallLatencyMs = simulation.getLastOverallLatencyMs();
    const double targetMs = simulation.getTimestep() * 1000.0;
    const double warningThresholdMs = simulation.getConfig().clockConfig.warningThreshold;
    const bool overallBehind = overallLatencyMs > targetMs + warningThresholdMs;
    const float instantaneousFps =
        renderLatencyMs > 0.0 ? static_cast<float>(1000.0 / renderLatencyMs) : 0.0f;

    ImGui::SeparatorText("Overall Latency");
    ImGui::TextDisabled("Active work only; excludes VSync and pacing delays");
    ImGui::Text("Overall Latency: %.3f ms", overallLatencyMs);
    ImGui::Text("Render Latency: %.3f ms", renderLatencyMs);
    ImGui::Text("Simulation Latency: %.3f ms", simulationLatencyMs);
    ImGui::Text("Status: %s", overallBehind ? "Behind" : "OK");
    ImGui::TextDisabled("Behind when active work > target (%.3f ms) + %.1f ms", targetMs,
                        warningThresholdMs);
    ImGui::Text("Target Timestep: %.3f ms (%d Hz)", targetMs,
                simulation.getConfig().clockConfig.targetSimulationFps);

    ImGui::SeparatorText("Render Latency");
    ImGui::TextDisabled("Draw work only; VSync wait excluded");
    ImGui::Text("Render Latency: %.3f ms", renderLatencyMs);
    ImGui::Text("Instantaneous FPS (Max FPS Potential): %.1f FPS", instantaneousFps);

    ImGui::SeparatorText("Simulation Latency");
    ImGui::TextDisabled("Time spent inside world.tick() on the last frame");
    ImGui::Text("Simulation Latency: %.3f ms", simulationLatencyMs);

    ImGui::Separator();

    bool debugClock = simulation.getConfig().DEBUG_CLOCK;
    if (ImGui::Checkbox("DEBUG_CLOCK console logging", &debugClock))
    {
        simulation.getConfig().DEBUG_CLOCK = debugClock;
        LogBuffer::instance().add(std::string("DEBUG_CLOCK ") +
                                  (debugClock ? "enabled" : "disabled") + ".");
    }
    ImGui::TextDisabled("Prints overall latency warnings to terminal");
    ImGui::Checkbox("Always show vehicle speeds", &showVehicleSpeeds);

    ImGui::End();
}

void drawWorldSummary(Simulation& simulation)
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.worldPosition, layout.worldSize);
    ImGui::Begin("World State", nullptr, ResponsivePanelFlags);

    const core::World& world = simulation.getWorld();

    ImGui::Text("Vehicles: %zu", world.getVehicleCount());
    ImGui::Text("Abstract Nodes: %zu", world.getAbstractNodeCount());
    ImGui::Text("Abstract Segments: %zu", world.getAbstractSegmentCount());
    ImGui::Text("Abstract Routes: %zu", world.getAbstractRouteCount());
    ImGui::Separator();
    ImGui::Text("Simulation Running: %s", simulation.isRunning() ? "Yes" : "No");
    ImGui::Text("Speed Multiplier: %.2fx", simulation.getSpeedMultiplier());
    ImGui::Text("Tick Count: %llu", static_cast<unsigned long long>(world.getTickCount()));
    ImGui::Text("Simulation Time: %.2f s", simulation.getSimTime());

    ImGui::End();
}

void drawEditorPanel(Simulation& simulation)
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.abstractNetworkPosition, layout.abstractNetworkSize);
    ImGui::Begin("Editor", nullptr, ResponsivePanelFlags);

    core::World& world = simulation.getWorld();
    if (world.getActiveRoute().has_value())
    {
        const AbstractRouteId routeId = *world.getActiveRoute();
        const network::AbstractRoute& route = world.getRoute(routeId);
        const network::RouteColor& color = route.getColor();

        ImGui::Text("Route");
        ImGui::Separator();
        ImGui::Text("Name: %s", route.getName().c_str());
        ImGui::Text("Nodes: %zu", route.getNodes().size());
        ImGui::ColorButton("Route Color", ImVec4(color.r, color.g, color.b, 1.0f));
        ImGui::SameLine();
        if (whiteButton("Edit Route"))
        {
            requestRouteEdit(world, routeId);
        }
        ImGui::SameLine();
        if (whiteButton("Delete Route"))
        {
            requestRouteDeletion(routeId);
        }
        ImGui::Text("ID: %d:%d", routeId.id, routeId.generation);
    }
    else if (!world.getActiveNode().has_value())
    {
        ImGui::TextDisabled("No object selected");
    }
    else
    {
        const AbstractNodeId nodeId = *world.getActiveNode();
        const network::AbstractNode& node = world.getNode(nodeId);
        const network::PhysicalNode& physicalNode = world.getPhysicalNode(nodeId);
        const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();

        ImGui::Text("Node");
        ImGui::Separator();
        ImGui::Text("Name: %s", node.getName().c_str());
        ImGui::Text("Type: %s", network::nodeTypeName(physicalNode.getNodeType()));
        ImGui::Text("Coordinate: (%.0f, %.0f, %.0f) m", coordinate.x, coordinate.y, coordinate.z);
        ImGui::Text("ID: %d:%d", nodeId.id, nodeId.generation);
    }

    ImGui::End();
}

void drawNodeTab(Simulation& simulation)
{
    core::World& world = simulation.getWorld();

    ImGui::Separator();

    if (ImGui::BeginTable(
            "Nodes", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX,
            ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing())))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("X (m)", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Y (m)", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Z (m)", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();

        world.forEachNode(
            [&](AbstractNodeId id, network::AbstractNode& node)
            {
                const network::PhysicalNode& physicalNode = world.getPhysicalNode(id);
                const network::PhysicalCoordinate& coordinate = physicalNode.getCoordinate();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(node.getName().c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(network::nodeTypeName(physicalNode.getNodeType()));
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.2f", coordinate.x);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", coordinate.y);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f", coordinate.z);
            });

        ImGui::EndTable();
    }

    ImGui::Text("Active nodes: %zu", world.getAbstractNodeCount());

    // if (ImGui::BeginTable("Nodes", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
    //     ImGui::TableSetupColumn("Name");
    //     ImGui::TableSetupColumn("ID");
    //     ImGui::TableSetupColumn("Max Speed (km/h)");
    //     ImGui::TableSetupColumn("Accel (km/h/s)");
    //     ImGui::TableSetupColumn("Decel (km/h/s)");
    //     ImGui::TableSetupColumn("Capacity");
    //     ImGui::TableSetupColumn("Actions");
    //     ImGui::TableHeadersRow();

    //     world.forEachVehicle([&](VehicleId id, vehicle::Vehicle& vehicle) {
    //         ImGui::TableNextRow();

    //         ImGui::TableSetColumnIndex(0);
    //         ImGui::TextUnformatted(vehicle.getDisplayName().c_str());

    //         ImGui::TableSetColumnIndex(1);
    //         ImGui::Text("%d:%d", id.id, id.generation);

    //         ImGui::TableSetColumnIndex(2);
    //         ImGui::Text("%.1f", vehicle.getMaxOperatingSpeedKph());

    //         ImGui::TableSetColumnIndex(3);
    //         ImGui::Text("%.1f", vehicle.getAccelerationKphPerSecond());

    //         ImGui::TableSetColumnIndex(4);
    //         ImGui::Text("%.1f", vehicle.getDecelerationKphPerSecond());

    //         ImGui::TableSetColumnIndex(5);
    //         ImGui::Text("%d", vehicle.getPassengerCapacity());

    //         ImGui::TableSetColumnIndex(6);
    //         const size_t vehicleIndex = static_cast<size_t>(id.id);
    //         std::ostringstream removeLabel;
    //         removeLabel << "Remove##" << id.id << "_" << id.generation;
    //         if (ImGui::SmallButton(removeLabel.str().c_str())) {
    //             world.removeVehicle(vehicleIndex);
    //             LogBuffer::instance().add(
    //                 "Removed vehicle \"" + vehicle.getDisplayName() + "\".");
    //         }
    //         });

    //     ImGui::EndTable();
    // }

    ImGui::Text("Total abstract nodes: %zu", world.getAbstractNodeCount());
}

void drawDebugViewer(Simulation& simulation)
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.vehiclePosition, layout.vehicleSize);
    ImGui::Begin("Debug Viewer", nullptr, ResponsivePanelFlags);

    if (ImGui::BeginTabBar("DebugViewerTabs"))
    {
        if (whiteTab("Vehicles"))
        {
            drawVehicleTab(simulation);
            ImGui::EndTabItem();
        }
        if (whiteTab("Nodes"))
        {
            drawNodeTab(simulation);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void drawEditorDialogs(Simulation& simulation)
{
    core::World& world = simulation.getWorld();
    drawVehicleCreationModal(world);
    drawNodeCreationModal(world);
    drawDeleteConfirmation(world);
    drawNodeMoveConfirmation(world);
}

void drawLogPanel()
{
    const PanelLayout layout = getPanelLayout();
    applyPanelLayout(layout.logPosition, layout.logSize);
    ImGui::Begin("Log", nullptr, ResponsivePanelFlags);

    if (whiteButton("Clear Log"))
    {
        LogBuffer::instance().clear();
    }

    ImGui::Separator();

    if (ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        for (const std::string& line : LogBuffer::instance().getLines())
        {
            ImGui::TextUnformatted(line.c_str());
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
} // namespace ui
