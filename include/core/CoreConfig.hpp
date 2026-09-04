#pragma once

struct ClockConfig
{
  static constexpr int TARGET_SIMULATION_FPS = 120;
  static constexpr double THREAD_SLEEP_VARIATION_ADJUSTMENT = 2.0;
  static constexpr double WARNING_THRESHOLD = 1.0;

  // Target simulation frames per second. Default provided for convenience.
  int targetSimulationFps = TARGET_SIMULATION_FPS;

  // Adjusts the sleep during time adjustment loop to account for OS delay (ms). Higher values
  // will result in higher CPU usage but less lag.
  double threadSleepVariationAdjustment = THREAD_SLEEP_VARIATION_ADJUSTMENT;

  // Threshold for logging a warning if the simulation is lagging behind (ms)
  double warningThreshold = WARNING_THRESHOLD;
};

struct NetworkConfig
{
};
struct GeometryConfig
{
};
struct VehicleConfig
{
};
struct PhysicsConfig
{
};
struct RenderingConfig
{
  // Visible square-map areas used by the physical grid renderer. The corresponding
  // side lengths are their square roots: 25 m² = 5 m x 5 m, 100 m² = 10 m x 10 m.
  double minimumGridZoomAreaSquareMeters = 25.0;
  // 25,000,000 m² gives a 5 km × 5 km outer square view.
  double maximumGridZoomAreaSquareMeters = 25000000.0;
  double defaultGridZoomAreaSquareMeters = 100.0;
};
struct UIConfig
{
  // Performance-panel latency sampling interval. This affects only the debug display, not
  // simulation or rendering work.
  double latencyDebugSampleIntervalSeconds = 0.125;
};
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
