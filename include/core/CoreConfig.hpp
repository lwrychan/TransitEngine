#pragma once


struct ClockConfig
{  // Target simulation frames per second. Default provided for convenience.
    int TARGET_SIMULATION_FPS = 120;

    // Adjusts the sleep during time adjustment loop to account for OS delay (ms). Higher values will result in higher CPU usage but less lag.
    double THREAD_SLEEP_VARIATION_ADJUSTMENT = 2.0;

    double WARNING_THRESHOLD = 1.0;  // Threshold for logging a warning if the simulation is lagging behind (ms)
};

struct NetworkConfig{};
struct GeometryConfig{};
struct VehicleConfig{};
struct PhysicsConfig{};
struct RenderingConfig{};
struct UIConfig{};
struct CoreConfig
{
    ClockConfig clockConfig;
    NetworkConfig networkConfig;
    GeometryConfig geometryConfig;
    VehicleConfig vehicleConfig;
    PhysicsConfig physicsConfig;
    RenderingConfig renderingConfig;
    UIConfig uiConfig;
    
    // ENABLE DEBUG PRINTING
    bool DEBUG_CLOCK = false;
};