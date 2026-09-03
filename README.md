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

The main view is a resizable, metre-based physical grid. It supports mouse-wheel zoom, middle-
mouse panning, adaptive grid detail, a scale bar, and configurable visible-area limits.

- **Pointer** selects, edits, and moves nodes; it also selects and edits routes.
- **Node** snaps the cursor to metre intersections and opens a creation dialog on click.
- **Route** creates an ordered route from selected existing nodes. Routes have names, colors,
  editable node sequences, labels, and confirmed deletion.
- Nodes have a logical `AbstractNode` and matching coordinate-bearing `PhysicalNode`. They can
  be named and classified as generic nodes, rail stations, bus stops, or road intersections.
- Labels avoid route geometry and other placed labels where possible. Selected and route-selected
  nodes use distinct highlight rings.

### Simulation

`Simulation` owns the SDL/ImGui frame loop. While running, it accumulates wall-clock time and
executes every required fixed simulation tick (120 Hz by default). At 1× speed, simulation time
therefore tracks elapsed real time; a speed multiplier scales simulated time without changing the
tick cadence.

- Vehicles are created with a display name, maximum operating speed, acceleration, deceleration,
  and passenger capacity.
- A vehicle can be assigned to an existing route and configured with local or endpoint service,
  then edited to select stops and dwell times per stop.
- Vehicle motion uses metre distances. It accelerates to the operating limit and uses braking
  distance to decelerate for the next stop or route end.
- Open routes reverse direction at their ends. Routes whose first and last nodes match are treated
  as closed loops.
- The grid draws direction-aligned vehicle markers. Hovering one shows its current speed; the
  debug view can keep speeds visible.

The optional debug view provides simulation controls, world counts, editor controls, logs, and
latency diagnostics. Render and simulation work are reported separately; overall latency is the
wall-clock frame duration and may include intentional frame pacing.

## Architecture

```text
main
  └─ Simulation
       ├─ core::World              editable runtime state and simulation rules
       │   ├─ logical nodes, routes, segments, and vehicles (generation-safe SlotMaps)
       │   ├─ PhysicalNode records (type and metre coordinates)
       │   ├─ AbstractNetwork      logical-topology boundary
       │   ├─ PhysicalNetwork      physical-segment-geometry boundary
       │   └─ MapNetwork           future schematic-projection boundary
       └─ render::Render
            ├─ menus, toolbar, normal/debug layout, and ImGui lifecycle
            ├─ ui::DebugPanels     creation/edit dialogs and diagnostics
            └─ render::grid
                 ├─ PhysicalGrid   grid panel composition and camera lifetime
                 ├─ GridCamera     world/screen transforms, panning, and zoom
                 ├─ GridBackdrop   adaptive grid and scale bar
                 ├─ GridInteraction tool input and drag behavior
                 ├─ GridScene      routes, nodes, vehicles, hover targets
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
in metres                        for a metro-style map
```

`AbstractNode` intentionally has no coordinate. `PhysicalNode` supplies the current editor's
node type and real-world coordinate. Today, `core::World` is the practical owner of the editable
node, route, vehicle, and physical-node containers. `AbstractNetwork` and `PhysicalNetwork`
express the intended boundary, but are not yet the canonical populated store: routes currently
contain ordered node IDs, their displayed geometry is straight node-to-node lines, and physical
segment geometry is reserved for the next implementation stage. `MapNetwork` is a placeholder.

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
- Add physical segment paths beyond straight node-to-node lines: curves, switches, elevation,
  geometry-aware lengths, and track constraints.
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

*Copyright © 2026 Lawrence Chan. All rights reserved.*
