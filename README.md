# TransitEngine

TransitEngine is a C++23 desktop prototype for building small transit networks and simulating
vehicles on them. SDL2, OpenGL, and Dear ImGui provide an interactive physical-grid editor;
the simulation advances the editable world at a fixed rate.

It is an editor and simulation foundation, not yet a full transit-planning application. The
sections below distinguish implemented behavior from the planned architecture.

## First-time setup and run

Requirements: CMake 3.28 or newer, a C++23 compiler, Git (for CMake `FetchContent`), and an
OpenGL-capable system.

```sh
cmake -S . -B build
cmake --build build
```

On macOS, CMake produces `build/TransitEngine.app`. On other platforms, run the generated
`TransitEngine` executable from the build directory.

Configuration downloads SDL2 2.32.10 and Dear ImGui 1.92.9, then copies `resources/` beside
or inside the application bundle.

## Current implementation

### Editor

The main view is a resizable, meter-based physical grid. It supports mouse-wheel zoom, middle-
mouse panning, adaptive grid detail, a scale bar, and configurable visible-area limits.

- **Pointer** selects, edits, and moves nodes; it also selects and edits routes.
- **Node** snaps the cursor to meter intersections and opens a creation dialog on click.
- **Route** creates an ordered route from selected existing nodes. Routes have names, colors,
  editable node sequences, labels, and confirmed deletion.
- **Geometry** edits the physical geometry layered over an active route. Intermediate geometry
  nodes can be added, moved, or removed; spans support linear or Bézier interpolation and an
  independent maximum speed.
- Nodes have a logical `AbstractNode` and matching coordinate-bearing `PhysicalNode`. They can
  be named and classified as generic nodes, rail stations, bus stops, or road intersections.
- Labels avoid route geometry and other placed labels where possible. Selected and route-selected
  nodes use distinct highlight rings.
- The grid has plan and axonometric 2.5D views. The latter renders elevation, vertical node
  guides, and vehicle prisms while keeping editing in the plan view.

### Simulation

`Simulation` owns the SDL/ImGui frame loop. While running, it accumulates wall-clock time and
executes every required fixed simulation tick (120 Hz by default). At 1× speed, simulation time
therefore tracks elapsed real time; a speed multiplier scales simulated time without changing the
tick cadence.

- Vehicles are created with a display name, maximum operating speed, acceleration, deceleration,
  and passenger capacity.
- A vehicle can be assigned to an existing route and configured with local or endpoint service,
  then edited to select stops and dwell times per stop.
- Vehicle motion uses physical route-geometry distance, including curves and elevation. It
  accelerates to the lower of its operating limit and the current geometry-span speed limit, then
  uses braking distance to decelerate for the next stop or route end.
- Open routes reverse direction at their ends. Routes whose first and last nodes match are treated
  as closed loops.
- The grid draws direction-aligned vehicle markers. Hovering one shows its current speed; the
  debug view can keep speeds visible.

Normal mode puts Simulation Controls, Inspector, and Editor in a fixed side column beside the
grid. Debug View adds Performance, World State, and Log panels. Performance readings are sampled
at an adjustable interval (8 Hz by default); this affects only the debug display. Render and
simulation work are reported separately; overall latency is the wall-clock frame duration and may
include intentional frame pacing.

## Tutorial: build and run a small route

This workflow exercises the editor and simulation features that are currently implemented.

1. **Navigate the grid.** Use the mouse wheel to zoom. Drag with the middle mouse button, or hold
   Left Alt while dragging with the left mouse button, to pan. Reset View returns to the origin
   when the map is empty, or frames the existing nodes. The 2.5D toolbar button changes only the
   presentation; edit nodes and geometry from the plan view.
2. **Create nodes.** Choose the Node tool, then click meter intersections on the grid. Give each
   node a name and type in the dialog, optionally adjusting its coordinates before Confirm. X and
   Y must remain inside the current 5 km grid extent (−2,500 m to +2,500 m); Z is available for
   elevation. Create at least two nodes.
3. **Create a route.** Choose the Route tool and click existing nodes in travel order. The bottom
   bar shows the selected-node count; choose Save Route, set a display name and color, then choose
   Save in the dialog. Clicking the first selected node again as the final selection creates a
   closed loop. Use the Pointer tool to select a route, or double-click/right-click it to reopen
   its editor.
4. **Shape its physical geometry.** With a route selected, choose Geometry. Click a geometry span
   to select it; the Editor panel then exposes its Linear/Bézier mode and maximum speed. Double-
   click a span to insert an intermediate geometry node, drag an intermediate node to reposition
   it, and right-click one to remove it. Curves, elevation, and span speed limits change the
   distance and speed used by vehicles.
5. **Create and assign a vehicle.** Open the Inspector's Vehicles tab and choose Create Vehicle,
   or choose Simulation → Manage Vehicles. Enter a name and positive operating values, or choose a
   preset, then Confirm. Reopen the route editor, select the vehicle under “Vehicles on this
   route,” and select either the Local or Express stop template. Individual node stops can be
   toggled, and each selected stop has an editable dwell time in seconds. Save the route to apply
   the assignment.
6. **Run the simulation.** Press Play in Simulation Controls (or use Simulation → Start / Resume).
   At 1×, simulation time follows wall-clock time at 120 updates per second. The vehicle
   accelerates, observes each geometry-span speed limit, brakes for selected stops, dwells for the
   configured time, and then continues. Open routes reverse at their ends; closed routes circulate.
   Use the speed multiplier to inspect behavior faster, and hover a vehicle to read its current
   speed.

## Architecture

```text
main
  └─ Simulation
       ├─ core::World              editable runtime state and simulation rules
       │   ├─ logical nodes, routes, segments, and vehicles (generation-safe SlotMaps)
       │   ├─ PhysicalNode records (type and meter coordinates)
       │   ├─ AbstractNetwork      logical-topology boundary
       │   ├─ PhysicalNetwork      physical-segment-geometry boundary
       │   └─ MapNetwork           future schematic-projection boundary
       └─ render::Render
            ├─ menus, toolbar, normal/debug layout, and ImGui lifecycle
            ├─ render::EditorLayout shared editor content bounds and gutters
            ├─ ui::DebugPanels     creation/edit dialogs and diagnostics
            └─ render::grid
                 ├─ PhysicalGrid   grid panel composition and camera lifetime
                 ├─ PhysicalGridViewport scene/backdrop/input coordination
                 ├─ GridCamera     world/screen transforms, panning, and zoom
                 ├─ GridBackdrop   adaptive grid and scale bar
                 ├─ GridInteraction tool input and drag behavior
                 ├─ GridScene      routes, nodes, vehicles, hover targets, and geometry markers
                 └─ GridLabels     readable unrotated and rotated labels
```

### Network layers

The model deliberately separates logical topology from its visual projections:

```text
AbstractNode / AbstractSegment / AbstractRoute
        geometry-free logical relationships
                         |
          +--------------+--------------+
          |                             |
PhysicalNetwork                  MapNetwork
real-world track geometry        future distorted schematic geometry
in meters                        for a metro-style map
```

`AbstractNode` intentionally has no coordinate. `PhysicalNode` supplies the current editor's
node type and real-world coordinate. Today, `core::World` is the practical owner of the editable
node, route, vehicle, and physical-node containers. Each route has an ordered abstract-node
sequence plus `PhysicalRouteGeometry`: anchored endpoint nodes, optional intermediate geometry
nodes, interpolation mode, and span speed limits. The geometry is the source for displayed route
curves and vehicle travel distance. `AbstractNetwork` and `PhysicalNetwork` still express the
intended boundary rather than a fully populated canonical store, while `MapNetwork` remains a
placeholder for schematic projection.

### Source layout

```text
include/
  core/          configuration and World
  network/       abstract topology and physical/map projection boundaries
  render/        SDL/OpenGL/ImGui renderer, tools, and modular grid renderer
  render/grid/   camera, backdrop, interactions, scene, labels, and panel composition
  ui/            dialogs, diagnostics, and shared styling
  vehicle/       vehicle state and movement model
  cli/           early terminal helper code
src/             implementations mirroring include/
resources/       fonts and application assets
```

Primary entry points are `src/main.cpp`, `src/Simulation.cpp`, `src/core/World.cpp`,
`src/render/Render.cpp`, `src/render/grid/PhysicalGrid.cpp`, and `src/ui/DebugPanels.cpp`.

## Planned features

### Canonical network and map model

- Populate `AbstractNetwork` as the canonical topology, then derive `PhysicalNetwork` and
  `MapNetwork` from it.
- Generate and persist `AbstractSegment` relationships when routes or track are created.
- Expand physical segment paths with switches, banks, track constraints, and richer elevation
  handling.
- Render a separate metro-style diagram from `MapNetwork` with intentionally distorted geometry.

### Physical-world editing

- Place buildings, terrain, and other grid obstructions.
- Validate track placement and route generation against obstructions and geometry constraints.
- Add map layers, richer selection tools, and persistence/import/export.

### Simulation depth

- Add signalling, blocks, reservations, switches, conflict prevention, and headway control.
- Model passenger demand, schedules, frequency, depots, and vehicle dispatch.
- Add curve speed limits, grades, additional vehicle types, scenarios, statistics, and automated
  tests for larger networks.

## Technology

- **C++23** — application and simulation code.
- **SDL2** — windowing, input, and OpenGL context.
- **Dear ImGui** — menus, modal editors, and diagnostics.
- **OpenGL 3.3** — renderer backend and map presentation.
- **CMake** — build configuration and dependency retrieval.

*Project created with AI assistance.*

*Copyright © 2026 Lawrence Chan. All rights reserved.*
