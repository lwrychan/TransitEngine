cmake_minimum_required(VERSION 3.28)

project(TransitEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10
)
FetchContent_MakeAvailable(SDL2)

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

set(SOURCES
    src/main.cpp
    src/core/World.cpp
    src/vehicle/Vehicle.cpp
    src/cli/display/ProgressBar.cpp
    src/cli/Terminal.cpp
    src/cli/View.cpp
    src/Resources.cpp
    src/Simulation.cpp
    src/render/Render.cpp
    src/render/grid/GridBackdrop.cpp
    src/render/grid/GridCamera.cpp
    src/render/grid/GridInteraction.cpp
    src/render/grid/GridLabels.cpp
    src/render/grid/GridScene.cpp
    src/render/grid/PhysicalGrid.cpp
    src/ui/DebugPanels.cpp
    src/network/Slot.cpp
    src/network/SlotMap.cpp
    src/network/PhysicalNode.cpp
)

if(APPLE)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/assets/icons/transit-engine-icon-1024.icns")

    add_executable(
        TransitEngine
        MACOSX_BUNDLE
        ${SOURCES}
        ${APP_ICON}
    )
else()
    add_executable(
        TransitEngine
        ${SOURCES}
    )
endif()

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

if(APPLE)
    add_custom_command(TARGET TransitEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_BUNDLE_CONTENT_DIR:TransitEngine>/Resources
    )

    set_source_files_properties(${APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(TransitEngine PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "TransitEngine"
        MACOSX_BUNDLE_ICON_FILE "transit-engine-icon-1024.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.lwrychan.transitengine"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    )

    target_sources(TransitEngine PRIVATE ${APP_ICON})

else()

    add_custom_command(TARGET TransitEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:TransitEngine>/resources
    )
endif()
