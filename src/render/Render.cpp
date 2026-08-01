#include "render/Render.hpp"
#include "Resources.hpp"

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
    std::cout << "Base (executable) directory: " << this->executableDirectory << std::endl;

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

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
}

bool render::Render::update() {
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
            render::Render::openURL("https://github.com/lwrychan/TransitEngine");
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