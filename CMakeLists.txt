cmake_minimum_required(VERSION 3.28)

project(TransitEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    transit-engine
    src/main.cpp
    src/core/World.cpp
    src/core/Clock.cpp
    src/vehicle/Vehicle.cpp
)

target_include_directories(
    transit-engine
    PRIVATE
    include
)
