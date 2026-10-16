
cmake_minimum_required(VERSION 3.20)

project(RoutingArena
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib 6.0 CONFIG REQUIRED)

add_executable(routing_arena
    src/main.cpp
)

target_link_libraries(routing_arena
    PRIVATE raylib
)
