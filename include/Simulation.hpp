#pragma once

#include "core/CoreConfig.hpp"
#include "core/Clock.hpp"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr int TARGET_FRAMERATE = 120;

class Simulation {
public:
    Simulation(const CoreConfig& config);

    static float pxFromPt(float pt, float dpi = 96.0f);

    void run();

private:
    CoreConfig globalConfig;
    double timestep;
    core::Clock clock;
    
    void openURL(const std::string& url);
};