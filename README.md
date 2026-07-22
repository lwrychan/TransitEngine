# Transit Simulation Engine

## Overview

C++-based simulation framework designed to model vehicles moving through a network of connected routes. The purpose of this engine is to allow for simulation of interconnected transportation systems such as transit or road networks and observe their efficiency.

This engine is designed with separation between simulation logic and visualization, allowing it to initially run as a standalone simulation while allowing flexible future expansion into a full graphical interface.

---

# Goals

* Support movement between nodes through network segments
* Provide extensible vehicle types (cars, buses, trains, etc.)
* Simulate vehicles moving through connected routes in real time
* Support realistic movement physics
* Allow future 2D and 3D visualization
* Enable route editing and interactive simulation control

---

# Architecture

```
Simulation Engine
│
├── Core
├── Network
├── Vehicles
├── Geometry
├── Physics
├── Render
└── UI
```

---

# Core

Contains the functionality for the simulation globally, including frame rate.

## CoreConfig

Stores configurable engine settings.

**Examples:**

* Target update rate
* Timestep length
* Simulation options

---

## World

The `World` class is the main container for the simulation state.

**Features:**

* Contains list of vehicles
* Contains network state
* Coordinate simulation updates
* Provide access to simulation state

---

## SimulationClock

Contains the simulation time state.

**Features:**

* Maintain global simulation time
* Control timestep updates
* Run the update loop
* Manage simulation speed

**Example:**

```
Simulation timestep:
120 FPS -> 8.333 ms per update
```

---

# Network

Represents the connections for transport lines as a network graph.

---

## Topology

Describes the connections between specific nodes in the network.

**Example:**

```
Station A
    |
Station B ---- Station D
    |
Station C
```

**Stores:**

* Nodes
* Connections
* Routes

---

## Geometry

Describes the physical shape of each segment connection between nodes.

**Examples:**

* Straight track
* Curved track
* Banked turns
* Elevation changes

**Potential attributes:**

* Length
* Coordinates
* Radius
* Curvature
* Banking angle

---

# Vehicles

Modes of transport with their own properties of maximum speed, acceleration, braking, etc.

The base `Vehicle` class contains common properties:

* Current segment
* Position along segment
* Speed
* Acceleration
* Passenger capacity

Example vehicle types:

* Car
* Bus
* Subway
* High-Speed Rail

---

# Physics

Processes the expected behavior of `Vehicle` objects given the track geometry and other factors.

**Vehicle properties:**

* Mass
* Maximum speed
* Maximum acceleration
* Maximum lateral acceleration

Physics calculations:

* Acceleration
* Braking
* Curve speed limits

**Example:**

A curved segment can calculate a safe speed based on:

* Curve radius
* Banking angle
* Vehicle characteristics

---

# Signalling

Responsible for managing vehicle movement conflicts and ensuring valid traversal of the network. It operates as an additional layer above the network, routes, and vehicles, using their current state to determine whether movement is permitted.

The network defines the static infrastructure, including nodes, tracks, and connections. Routes define the intended path through the network, while vehicles store their current position and progress along their assigned routes.

The signalling system is intended to handle:
- Track occupancy
- Vehicle spacing requirements
- Opposing vehicle movement conflicts
- Switch track permissions
- Movement restrictions and stopping conditions

When a vehicle attempts to enter or continue along a track, it queries the signalling system. The signalling system evaluates the surrounding vehicles, track properties, and route information to determine whether the vehicle can proceed, must reduce speed, or must stop.

Future extensions may include block signalling, signal states, track reservations, priority handling, etc.


# Rendering

**Display graphics for:**

* Routes
* Vehicles
* Stations
* Infrastructure

## Possible rendering modes ##
## 2D View

* Network map
* Route overview
* Vehicle positions

## 3D View

* Track elevation
* Banked curves
* Terrain

---

# User Interface (Future)

Provide user interaction to add and modify routes, vehicles, and track network / geometry.

Features:

* Start/stop simulation
* Adjust simulation speed
* Edit vehicle properties
* Edit routes
* Modify segment properties
* View statistics

---

# Implementation

## C++

C++ chosen for compatibility and performance.

---

## SDL3
* Window creation
* Input handling

---

## Dear ImGui
* Debug panels
* Simulation controls
* Editors
* Property inspectors

---

## OpenGL
* Map rendering
* 2D/3D visualization

---

# Build

Project build uses CMake.

```
Setup:
cmake -S . -B build

Build:
cmake --build build
```

Generated files are stored in the build directory and are not committed.

---

# Repository Structure

```
TransitEngine/

├── include/
│   ├── core/
│   ├── network/
│   ├── vehicles/
│   ├── geometry/
│   └── physics/

├── src/
│   ├── core/
│   ├── network/
│   ├── vehicles/
│   ├── geometry/
│   └── physics/

├── CMakeLists.txt
├── README.md
├── .gitignore
└── .gitattributes
```

---

# Future Development

Potential future features:

* Route editor
* Real-world map importing
* Multiple vehicle types
* Traffic simulation
* Passenger simulation
* Realistic train physics
* 3D visualization
* Scenario creation
* Performance optimization for large networks

---

# Design Principles

## Separation of concerns

Simulation, physics, geometry, and rendering remain independent.

## Extensibility

New vehicles, routes, and visualizations should be added without rewriting existing systems.

## Realism where useful

Physics models can become more detailed while maintaining a simple simulation interface.

## Data-driven design

Future systems should allow networks, vehicles, and scenarios to be loaded from external files rather than hard-coded.
# Transit Simulation Engine

## Overview

Transit Simulation Engine is a C++-based simulation framework designed to model vehicles moving through a network of connected routes. The goal is to provide a flexible foundation for simulating transportation systems such as rail networks, road networks, and other graph-based transit systems.

The engine is designed with separation between simulation logic and visualization, allowing it to initially run as a standalone simulation while supporting future expansion into a full graphical editor and visualization platform.

---

# Goals

* Simulate vehicles moving through connected routes in real time
* Support movement between nodes through network segments
* Provide extensible vehicle types (cars, buses, trains, etc.)
* Support realistic movement physics
* Allow future 2D and 3D visualization
* Enable route editing and interactive simulation control

---

# Core Architecture

The engine is divided into independent systems:

```
Simulation Engine
│
├── Core
├── Network
├── Vehicles
├── Geometry
├── Physics
├── Rendering
└── UI
```

Each system has a specific responsibility and communicates through well-defined interfaces.

---

# Core

The core system manages the simulation itself.

## World

The `World` class is the main container for the simulation state.

Responsibilities:

* Own vehicles
* Own the transportation network
* Coordinate updates
* Provide access to simulation state

---

## Simulation

The simulation controller manages time progression.

Responsibilities:

* Maintain global simulation time
* Control timestep updates
* Run the update loop
* Manage simulation speed

Example:

```
Simulation timestep:
120 updates per second
```

---

## CoreConfig

Stores configurable engine settings.

Examples:

* Target update rate
* Timestep length
* Simulation options

---

# Network System

The network system represents the structure that vehicles travel through.

The network is separated into topology and geometry.

---

## Topology

Topology describes connectivity.

It answers:

"Which locations are connected?"

Example:

```
Station A
    |
Station B ---- Station D
    |
Station C
```

The topology contains:

* Nodes
* Connections
* Routes

---

## Geometry

Geometry describes physical shape.

It answers:

"What does the route look like?"

Examples:

* Straight track segments
* Curved segments
* Transition curves
* Future 3D terrain

A segment may contain:

* Length
* Coordinates
* Radius
* Curvature
* Banking angle

The simulation does not depend on how a segment looks. It only needs to know how far a vehicle has travelled along it.

---

# Vehicles

Vehicles represent objects moving through the network.

The base `Vehicle` class contains common properties:

* Current segment
* Position along segment
* Speed
* Acceleration
* Vehicle capabilities

Example vehicle types:

* Car
* Bus
* Train

Specialized vehicles inherit from the base vehicle class.

---

# Physics System

The physics system calculates how vehicles behave.

The vehicle stores capabilities, while physics determines results.

Vehicle properties:

* Mass
* Maximum speed
* Maximum acceleration
* Maximum lateral acceleration

Physics calculations:

* Acceleration
* Braking
* Curve speed limits
* Effects of banking
* Movement updates

Example:

A curved segment can calculate a safe speed based on:

* Curve radius
* Banking angle
* Vehicle characteristics

---

# Rendering System (Future)

The renderer is independent from the simulation.

The simulation determines:

* Where vehicles are
* What routes exist
* Current state

The renderer displays:

* Routes
* Vehicles
* Stations
* Infrastructure

Possible rendering modes:

## 2D View

* Network map
* Route overview
* Vehicle positions

## 2.5D / 3D View

* Track elevation
* Banked curves
* Terrain
* Realistic visualization

---

# User Interface (Future)

The UI will provide interaction with the simulation.

Possible features:

* Start/stop simulation
* Adjust simulation speed
* Inspect vehicles
* Edit routes
* Modify segment properties
* View statistics

The UI will communicate with the engine rather than directly controlling simulation logic.

---

# Planned Libraries

## C++

Main implementation language.

Advantages:

* High performance
* Low-level control
* Strong ecosystem for simulation and graphics
* Cross-platform support

---

## SDL3

Used for:

* Window creation
* Input handling
* Basic rendering support

---

## Dear ImGui

Used for:

* Debug panels
* Simulation controls
* Editors
* Property inspectors

---

## OpenGL (Future)

Used for:

* Advanced rendering
* 3D visualization
* Hardware accelerated graphics

---

# Build System

The project uses CMake.

Example workflow:

```
Configure:
cmake -S . -B build

Build:
cmake --build build
```

Generated files are stored in the build directory and are not committed.

---

# Repository Structure

```
TransitEngine/

├── include/
│   ├── core/
│   ├── network/
│   ├── vehicles/
│   ├── geometry/
│   └── physics/

├── src/
│   ├── core/
│   ├── network/
│   ├── vehicles/
│   ├── geometry/
│   └── physics/

├── CMakeLists.txt
├── README.md
├── .gitignore
└── .gitattributes
```

---

# Future Development

Potential future features:

* Route editor
* Real-world map importing
* Multiple vehicle types
* Traffic simulation
* Passenger simulation
* Realistic train physics
* 3D visualization
* Scenario creation
* Performance optimization for large networks

---

# Design Principles

## Separation of concerns

Simulation, physics, geometry, and rendering remain independent.

## Extensibility

New vehicles, routes, and visualizations should be added without rewriting existing systems.

## Realism where useful

Physics models can become more detailed while maintaining a simple simulation interface.

## Data-driven design

Future systems should allow networks, vehicles, and scenarios to be loaded from external files rather than hard-coded.


*Copyright © 2026 Lawrence Chan* \
*All rights reserved.*