# ========================
# Setup
# ========================

cmake_minimum_required(VERSION 3.28)

project(TransitEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


# ========================
# Dependencies
# ========================

FetchContent_Declare( # Define dependency
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10
)
FetchContent_MakeAvailable(SDL2) # Download SDL2

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9
)
FetchContent_MakeAvailable(imgui)

add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
)

target_link_libraries(imgui PUBLIC
    SDL2::SDL2
    OpenGL::GL
)

find_package(OpenGL REQUIRED)


# ========================
# Executables
# ========================
add_executable(
    TransitEngine
    src/main.cpp
    src/core/World.cpp
    src/core/Clock.cpp
    src/vehicle/Vehicle.cpp
    src/vehicle/rail/RailVehicle.cpp
    src/cli/display/ProgressBar.cpp
    src/cli/Terminal.cpp
    src/cli/View.cpp
    src/Simulation.cpp
)

target_include_directories(
    TransitEngine
    PRIVATE
    include
)

target_link_libraries(
    TransitEngine
    PRIVATE
    imgui
)

add_custom_command(TARGET TransitEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/fonts
    $<TARGET_FILE_DIR:TransitEngine>/fonts
)
