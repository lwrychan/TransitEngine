# TransitEngine

TransitEngine is a C++23 desktop prototype for editing a small transit network and simulating vehicles moving along its routes. It uses SDL2, OpenGL, and Dear ImGui to combine an interactive physical-grid editor with a fixed-timestep simulation loop.

The project is currently an editor and simulation foundation, not a complete transit-planning application. This document distinguishes what works today from the planned architecture and features.

## First-time setup and run

Requirements: CMake 3.28 or newer, a C++23 compiler, Git (for CMake FetchContent), and an OpenGL-capable system.

```sh
cmake -S . -B build
cmake --build build
```

On macOS, CMake builds an application bundle; launch `build/TransitEngine.app`. On other platforms, run the generated `TransitEngine` executable from the build directory.

CMake downloads SDL2 2.32.10 and Dear ImGui 1.92.9 during configuration, then copies `resources/` beside or inside the resulting application.

## Current implementation

### Interactive editor

The application opens a resizable physical-grid map measured in metres. The view can be panned with the pointer tool and zoomed with the mouse wheel; the grid adapts its detail and has configurable minimum, maximum, and default visible areas.

- Create physical nodes by selecting the **Node** tool and clicking a grid point. The cursor snaps to grid intersections while the tool is active.
- Name and classify nodes as generic nodes, rail stations, bus stops, or road intersections; edit, move, select, and delete them through dialogs.
- Create routes from an ordered selection of existing nodes, select them on the map, edit their name, node sequence, and color, or delete them with confirmation.
- Render route polylines, route labels, station labels, selection rings, and vehicle markers on the physical grid.
- Create vehicles with a display name, maximum speed, acceleration, deceleration, and capacity. Assign existing vehicles to routes and edit their stopping pattern and dwell time.

### Simulation

`Simulation` owns the main SDL/ImGui render loop and advances `core::World` at a configurable fixed timestep (120 FPS by default) when the simulation is running or a single step is requested.

- Simulation speed can be paused, stepped, or scaled from the UI.
- Each vehicle travels in real-world metres, accelerates toward its operating-speed limit, and brakes for its next stopping control point.
- Stopped vehicles dwell for the configured time. Default stop templates include local service (every route node) and express service (endpoints), with per-stop editing.
- Vehicles traverse multi-node routes. Open routes reverse direction at their ends; a route whose first and last node are the same is treated as a closed loop.
- Vehicle speed can be inspected on hover or shown persistently from the debug UI.

The debug view is optional and contains simulation controls, world counts, latency diagnostics, the editor inspector, and an event log. The normal view retains the map, toolbar, menus, and necessary creation/edit dialogs.

### Current data model

The naming separates logical topology from projections that will represent it differently:

```text
AbstractNode / AbstractSegment / AbstractRoute
        logical topology; no physical coordinates on AbstractNode
                         |
                         +--> PhysicalNetwork / PhysicalNode
                         |    real-world geometry in metres
                         |
                         +--> MapNetwork
                              future diagrammatic (metro-map) projection
```

Today, `core::World` is the owner of the editable runtime state. It keeps active nodes, routes, and vehicles in generation-safe `SlotMap` containers; it also owns a segment container reserved for the upcoming segment-generation path. Matching `PhysicalNode` objects hold node type and coordinates. The current grid draws straight lines between the physical coordinates of the nodes in each route, and vehicle positions are interpolated along those lines.

`AbstractNetwork`, `PhysicalNetwork`, and `MapNetwork` define the intended boundaries, but they are not yet the sole populated source of editor data: `PhysicalNetwork` currently only provides storage for future per-segment geometry, and `MapNetwork` is an empty placeholder. This is deliberate unfinished work, listed below, rather than functionality already supplied by the editor.

### Source layout

```text
include/
  core/       configuration and World, the runtime owner
  network/    abstract topology, physical nodes/geometry, map projection boundary
  render/     SDL/OpenGL/ImGui renderer and physical-grid view
  ui/         editor dialogs, debug panels, shared UI styling
  vehicle/    vehicle state and movement model
  cli/        terminal helpers retained from the early prototype
src/          implementations matching the modules above
resources/    fonts and application assets
```

Key entry points are `src/main.cpp`, `src/Simulation.cpp`, `src/core/World.cpp`, `src/render/Render.cpp`, `src/render/PhysicalGridView.cpp`, and `src/ui/DebugPanels.cpp`.

## Planned features

These capabilities are intentionally not represented as complete in the current codebase.

### Network and map architecture

- Make `AbstractNetwork` the populated canonical topology and derive both `PhysicalNetwork` and `MapNetwork` from it.
- Create and persist `AbstractSegment` relationships when routes or track are generated.
- Support physical segment geometry beyond straight node-to-node lines: curves, switches, track alignment, elevation, and geometry-aware lengths.
- Add a simplified, distorted `MapNetwork` renderer for metro-style diagrams separate from the physical-grid map.
- Generate and edit route segments from selected nodes through a dedicated route-generation workflow.

### Physical world editing

- Place custom buildings, terrain, and other grid obstructions.
- Validate track placement and route generation against obstructions and geometry constraints.
- Add richer map tools, layers, selection behaviors, and persistence/import/export of network data.

### Simulation depth

- Add signalling, block occupancy, reservations, switches, conflict prevention, and headway control.
- Model passengers, demand, timetables, service frequency, depots, and vehicle dispatch.
- Expand vehicle families and route/track constraints, including curve speed limits, grades, and more complete train physics.
- Improve the timing loop to preserve accumulated elapsed time during slow frames rather than advancing at most one fixed tick per iteration.
- Add scenario tooling, statistics, automated tests, and performance work for large networks.

### Presentation

- Add a true map-diagram mode backed by `MapNetwork`.
- Provide 3D or terrain-aware rendering only after the physical geometry model supports it.
- Continue separating normal editor UI from diagnostic tooling and consolidate the remaining large editor/render functions as the feature set grows.

## Technology

- **C++23** — simulation and application code.
- **SDL2** — windowing, input, and OpenGL context.
- **Dear ImGui** — menus, editors, modal dialogs, and diagnostics.
- **OpenGL 3.3** — ImGui rendering backend and map presentation.
- **CMake** — build configuration and dependency retrieval.

*Copyright © 2026 Lawrence Chan. All rights reserved.*
