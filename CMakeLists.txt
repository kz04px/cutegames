cmake_minimum_required(VERSION 3.12)

# Project
project(cutegames VERSION 1.0 LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/)

find_package(Threads REQUIRED)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -pedantic -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add cutegames executable
add_executable(
    cutegames
    src/main.cpp
    src/openings.cpp
    src/play.cpp
    src/settings.cpp
    # Engine
    src/engine/engine.cpp
    # Events
    src/events/on_engine_loaded.cpp
    src/events/on_engine_unloaded.cpp
    src/events/on_game_finished.cpp
    src/events/on_game_started.cpp
    src/events/on_match_finished.cpp
)

# Add the test executable
add_executable(
    tests
    tests/main.cpp
    tests/events.cpp
    tests/store.cpp
)

target_link_libraries(
    cutegames
    Threads::Threads
)

set_property(TARGET tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
