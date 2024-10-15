cmake_minimum_required(VERSION 3.14)

# Project
project(cutegames VERSION 1.0 LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG 63258397761b3dd96dd171e5a5ad5aa915834c35
)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG ae7a13539fb71f270b87eb2e874fbac80bc8dda2
)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG e52bef1eb889aab0cdc7dc0d40d1e83ffbd06af6
)

FetchContent_Declare(
    termcolor
    GIT_REPOSITORY https://github.com/ikalnytskyi/termcolor
    GIT_TAG 89f20096bef51de347ec6f99345f65147359bd7c
)

FetchContent_Declare(
    libataxx
    GIT_REPOSITORY https://github.com/kz04px/libataxx
    GIT_TAG 4226c26dd11a1f74be708882ece6fd9dc96c767b
)

FetchContent_Declare(
    libchess
    GIT_REPOSITORY https://github.com/kz04px/libchess
    GIT_TAG 71c3543da7ad64387b618e889feef5406f1ee851
)

FetchContent_MakeAvailable(json doctest cli11 termcolor libataxx libchess)

find_package(Threads REQUIRED)

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add cutegames executable
add_executable(
    cutegames

    src/main.cpp

    # Match
    src/match/openings.cpp
    src/match/pgn.cpp
    src/match/play.cpp
    src/match/settings.cpp

    # Engine
    src/engine/engine_process.cpp
    src/engine/engine_uai.cpp
    src/engine/engine_uci.cpp
    src/engine/engine_ugi.cpp

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
    tests/elo.cpp
    tests/sprt.cpp

    # Games
    tests/games/ataxx.cpp
    tests/games/chess.cpp
    tests/games/generic.cpp

    # Tournaments
    tests/tournament/gauntlet.cpp
    tests/tournament/roundrobin.cpp

    # CuteGames
    src/match/play.cpp
)

target_link_libraries(
    cutegames
    Threads::Threads
    CLI11::CLI11
    nlohmann_json::nlohmann_json
    termcolor::termcolor
    ataxx_static
    libchess_static
)

target_link_libraries(
    tests
    doctest::doctest
    ataxx_static
    libchess_static
)

set_property(TARGET tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
