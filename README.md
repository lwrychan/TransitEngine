# TransitEngine

## Overview

C++-based simulation framework designed to model vehicles moving through a network of connected routes. The purpose of this engine is to allow for simulation of interconnected transportation systems such as transit or road networks and observe their efficiency.

This engine is designed with separation between simulation logic and visualization, allowing it to initially run as a standalone simulation while allowing flexible future expansion into a full graphical interface.

---

# Setup

**First Time Setup**
```
cd TransitEngine         Navigate to directory
cmake -S . -B build      Generate build/ folder
cd build/                Navigate to build/ directory
cmake --build .          Build project to build/
```

**Run program**
```
* ./TransitEngine          Run program
```

# Goals

* Support movement between nodes through network segments
* Provide extensible vehicle types (cars, buses, trains, etc.)
* Simulate vehicles moving through connected routes in real time
* Support realistic movement physics
* Allow future 2D and 3D visualization
* Enable route editing and interactive simulation control

---

# Project Structure

```
TransitEngine/
├── CMakeLists.txt           # CMakeLists
├── README.md
├── build/                   # Build / compiled output, dependencies
├── fonts/                   # Font assets
├── include/                 # Header files
│   ├── Simulation.hpp
│   ├── cli/
│   ├── core/
│   ├── geometry/
│   ├── network/
│   ├── physics/
│   ├── render/
│   ├── signalling/
│   ├── ui/
│   └── vehicle/
├── src/                     # Implementations
│   ├── main.cpp
│   ├── Simulation.cpp
│   ├── cli/
│   ├── core/
│   ├── network/
│   └── vehicle/
└── tests/
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

# User Interface

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

## SDL2
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

# Project Structure

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

## Separation of features

Simulation, physics, geometry, and rendering are separate modules and can be developed independently.

## Data-driven design

Future systems should allow networks, vehicles, and scenarios to be loaded from external files rather than hardcoded.


*Copyright © 2026 Lawrence Chan* \
*All rights reserved.*