#pragma once

#include <SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL_opengl.h>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <iostream>

class Simulation;

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr int TARGET_FRAMERATE = 120;

namespace render {
    class Render {
    public:
        void setup();
        bool update(Simulation& simulation);
        void close();

        float pxFromPt(float pt, float dpi = 96.0);
        void openURL(const std::string& url);

        double getLastRenderWorkMs() const;

    private:
        SDL_GLContext glContext;
        SDL_Window* window;

        std::filesystem::path executableDirectory;
        std::filesystem::path resourcesPath;
        std::filesystem::path fontResourcesPath;

        double lastRenderWorkMs = 0.0;
    };
}