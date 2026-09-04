#pragma once

#include "core/CoreConfig.hpp"
#include "core/World.hpp"

class Simulation
{
  public:
  Simulation(const CoreConfig& config);

  void run();

  core::World& getWorld();
  const core::World& getWorld() const;

  CoreConfig& getConfig();
  const CoreConfig& getConfig() const;

  bool isRunning() const;
  void setRunning(bool running);
  void toggleRunning();

  double getSpeedMultiplier() const;
  void setSpeedMultiplier(double multiplier);

  void requestStepOnce();

  double getSimTime() const;
  double getTimestep() const;

  // Active render and simulation work for the last frame. Excludes VSync and pacing sleep.
  double getLastRenderLatencyMs() const;

  // Time spent executing world.tick() on the last frame (0 if no tick ran).
  double getLastSimulationLatencyMs() const;

  // Combined active work for the last frame. Excludes artificial pacing delays.
  double getLastOverallLatencyMs() const;

  private:
  CoreConfig globalConfig;
  double timestep;
  bool simulationRunning;
  bool stepOnceRequested;
  double speedMultiplier;
  double simTime;
  double lastRenderLatencyMs;
  double lastSimulationLatencyMs;
  double lastOverallLatencyMs;
  core::World world;
};
