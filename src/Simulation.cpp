#include <SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL_opengl.h>
#include <cstdlib>
#include <string>

#include "Simulation.hpp"

Simulation::Simulation(const CoreConfig& config)
    : globalConfig(config),
    timestep(1.0 / globalConfig.clockConfig.targetSimulationFps),
    clock(globalConfig.clockConfig.targetSimulationFps) {
}

float Simulation::pxFromPt(float pt, float dpi) {
    // With 96 DPI: px = pt * DPI / 72. ~ px = pt * 4/3
    return pt * dpi / 72;
}

void Simulation::openURL(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#elif __linux__
    std::string command = "xdg-open " + url;
#endif
    std::system(command.c_str());
}

void Simulation::run() {
    // Initial SDL2, ImGui, and OpenGL setup
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "Transit Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // VSync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // IO Settings
    ImGuiIO& ioSettings = ImGui::GetIO();
    ioSettings.FontGlobalScale = 1.0f;

    // Size in px
    ioSettings.Fonts->AddFontFromFileTTF(
        "fonts/GoogleSansFlex.ttf",
        Simulation::pxFromPt(16.0)
    );
    // =============

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");


    bool running = true;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // ============
        // UI Elements
        // ============

        ImGui::Begin("Transit Engine");

        ImGui::Text("Hello World!");

        if (ImGui::Button("Pause"))
        {
        }

        ImGui::End();

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(
            ImVec2(viewport->Size.x, 40)
        );

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse;

        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("About"))
        {
            if (ImGui::MenuItem("About Transit Engine"))
            {
                // About
            }
            if (ImGui::MenuItem("GitHub Repository"))
            {
                Simulation::openURL("https://github.com/lwrychan/TransitEngine");
            }

            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Simulation"))
        {
            if (ImGui::MenuItem("Start / Resume"))
            {
                // start simulation
            }

            if (ImGui::MenuItem("Pause"))
            {
                // Pause
            }

            if (ImGui::BeginMenu("Simulation Rate"))
            {
                if (ImGui::MenuItem("1x"))
                {
                    // Set simulation speed to 1x;
                }

                if (ImGui::MenuItem("2x"))
                {
                    // Set simulation speed to 2x;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Vehicle"))
            {
                if (ImGui::MenuItem("Car")) {}
                if (ImGui::MenuItem("Truck")) {}
                if (ImGui::MenuItem("Light Rail")) {}
                if (ImGui::MenuItem("Subway")) {}
                if (ImGui::MenuItem("Regional Train")) {}
                if (ImGui::MenuItem("HSR")) {}

                ImGui::EndMenu();
            }


            if (ImGui::BeginMenu("Node"))
            {
                if (ImGui::BeginMenu("NodeMenu")) {
                    if (ImGui::MenuItem("Rail Station")) {}
                    if (ImGui::MenuItem("Bus Stop")) {}
                    if (ImGui::MenuItem("Road Intersection")) {}

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Undo");
            ImGui::MenuItem("Redo");

            ImGui::MenuItem("Copy");
            ImGui::MenuItem("Paste");

            ImGui::MenuItem("Rotate Clockwise");
            ImGui::MenuItem("Rotate Counterclockwise");
            ImGui::MenuItem("Set Rotation");

            ImGui::MenuItem("Properties");

            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Routes");
            ImGui::MenuItem("Show Vehicles");

            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Documentation"))
            {
            }

            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();

        ImGui::Render();

        // OpenGL rendering

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    SDL_Quit();
}


// TimePoint iterationStart = std::chrono::high_resolution_clock::now();
//     this->clock.step();

//     // Initial setup
//     this->setup();

//     while (true)
//     {
//         TimePoint iterationEnd = std::chrono::high_resolution_clock::now();

//         double iterationTime = std::chrono::duration<double>(iterationEnd - iterationStart).count();

//         if (iterationTime >= this->timestep)
//         {
//             if (this->globalConfig.DEBUG_CLOCK)
//             {
//                 std::cout << "Iteration time: " << iterationTime * 1000 << " ms" << std::endl;
//             }

//             iterationStart = std::chrono::high_resolution_clock::now();

//             // Check for simulation processing time
//             TimePoint stepStart = std::chrono::high_resolution_clock::now();

//             // Run step for all simulation modules here
//             this->tick();

//             TimePoint stepEnd = std::chrono::high_resolution_clock::now();
//         }
//         else
//         {

//             std::this_thread::sleep_for(std::chrono::duration<double>((this->timestep - iterationTime) - (this->globalConfig.clockConfig.THREAD_SLEEP_VARIATION_ADJUSTMENT * 1e-3)));
//         }

//         if (this->globalConfig.DEBUG_CLOCK)
//         {
//             // Check for 1 ms deviation from expected timestep and log a warning if the simulation is lagging behind
//             if (iterationTime - this->timestep > this->globalConfig.clockConfig.warningThreshold * 1e-3)
//             {
//                 std::cout << "WARNING || Simulation step took longer than target timestep. Currently lagging behind by " << (iterationTime - this->timestep) * 1000 << " ms" << std::endl;
//             }
//         }
//     }
