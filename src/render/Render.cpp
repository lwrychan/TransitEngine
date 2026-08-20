#include "render/Render.hpp"
#include "Resources.hpp"
#include "Simulation.hpp"
#include "render/PhysicalGridView.hpp"
#include "ui/DebugPanels.hpp"

#include <chrono>

float  render::Render::pxFromPt(float pt, float dpi) {
    // With 96 DPI: px = pt * DPI / 72. ~ px = pt * 4/3
    return pt * dpi / 72;
}

void render::Render::openURL(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#elif __linux__
    std::string command = "xdg-open " + url;
#endif
    std::system(command.c_str());
}

void render::Render::setup() {
    Resources resources = Resources();
    // Initial SDL2, ImGui, and OpenGL setup
    SDL_Init(SDL_INIT_VIDEO);

    char* basePath = SDL_GetBasePath();
    this->resourcesPath = std::filesystem::path(basePath ? basePath : "");

    SDL_free(basePath);

    this->executableDirectory = this->resourcesPath / ".." / "MacOS";
    this->fontResourcesPath = this->resourcesPath / "fonts" / "GoogleSansFlex.ttf";


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    this->window = SDL_CreateWindow(
        "Transit Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    this->glContext = SDL_GL_CreateContext(this->window);

    SDL_GL_MakeCurrent(this->window, this->glContext);
    SDL_GL_SetSwapInterval(1); // VSync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Window and ImGui context created.
    std::cout << "Window and ImGui context created.";
    std::cout << "Base (executable) directory: " << this->executableDirectory << std::endl;

    ImGui::StyleColorsLight();
    // ImGui::StyleColorsDark();
    
    std::cout << "Set background to light theme.";

    // IO Settings
    std::cout << "Loading fonts...\n" << std::endl;

    ImGuiIO& ioSettings = ImGui::GetIO();
    ioSettings.FontGlobalScale = 1.0f;
    ioSettings.Fonts->AddFontFromFileTTF(
        this->fontResourcesPath.string().c_str(),
        render::Render::pxFromPt(16.0)
    );

    std::cout << "Resource directory: " << this->resourcesPath << std::endl;
    // =============

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    ui::LogBuffer::instance().add("Transit Engine initialized.");
}

double render::Render::getLastRenderWorkMs() const {
    return this->lastRenderWorkMs;
}

bool render::Render::update(Simulation& simulation) {
    const auto renderWorkStart = std::chrono::high_resolution_clock::now();

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
            return false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    bool showPopupManageVehicles = false;

    if (ImGui::BeginMainMenuBar())
    {
    if (ImGui::BeginMenu("About"))
    {
        if (ImGui::MenuItem("About Transit Engine"))
        {
            // About
        }
        if (ImGui::MenuItem("GitHub Repository"))
        {
            render::Render::openURL("https://github.com/lwrychan/TransitEngine");
        }

        ImGui::EndMenu();
    }


    if (ImGui::BeginMenu("Simulation"))
    {
        if (ImGui::MenuItem("Start / Resume"))
        {
            simulation.setRunning(true);
            ui::LogBuffer::instance().add("Simulation started from menu.");
        }

        if (ImGui::MenuItem("Pause"))
        {
            simulation.setRunning(false);
            ui::LogBuffer::instance().add("Simulation paused from menu.");
        }

        if (ImGui::BeginMenu("Simulation Rate"))
        {
            if (ImGui::MenuItem("0.25x"))
            {
                simulation.setSpeedMultiplier(0.25);
            }

            if (ImGui::MenuItem("1x"))
            {
                simulation.setSpeedMultiplier(1.0);
            }

            if (ImGui::MenuItem("2x"))
            {
                simulation.setSpeedMultiplier(2.0);
            }

            if (ImGui::MenuItem("4x"))
            {
                simulation.setSpeedMultiplier(4.0);
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Manage Vehicles")) {
            showPopupManageVehicles = true;
        }

        ImGui::EndMenu();
    }


    if (ImGui::BeginMenu("Add"))
    {
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
    }

    ui::drawSimulationControls(simulation);
    ui::drawVehicleInspector(simulation);
    ui::drawPerformancePanel(simulation);
    ui::drawWorldSummary(simulation);
    render::drawPhysicalGridView(
        simulation.getWorld().getPhysicalNetwork(), simulation.getConfig().renderingConfig);
    ui::drawNetworkInspector(simulation);
    ui::drawLogPanel();

    if (ImGui::BeginPopupModal("Manage Vehicles", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Vehicle management");
        ImGui::Text("Use the Vehicle Inspector panel to add or remove vehicles.");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            // Execute reset logic here
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (showPopupManageVehicles) {
        ImGui::OpenPopup("Manage Vehicles");
    }

    ImGui::Render();

    // OpenGL rendering

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    const auto renderWorkEnd = std::chrono::high_resolution_clock::now();
    this->lastRenderWorkMs =
        std::chrono::duration<double>(renderWorkEnd - renderWorkStart).count() * 1000.0;

    SDL_GL_SwapWindow(window);

    return true;
}

void render::Render::close() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();

    SDL_GL_DeleteContext(this->glContext);
    SDL_DestroyWindow(this->window);

    SDL_Quit();
}
