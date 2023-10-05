cmake_minimum_required(VERSION 3.10)  # Specify the minimum CMake version required

project(GravitySimulation)  # Name your project

set(CMAKE_CXX_STANDARD 17)  # Specify the C++ standard

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)  # Find SFML

add_executable(GravitySimulation src/Main.cpp src/Particle.cpp src/Quadtree.cpp src/Timer.cpp)  # Specify the executable and its sources

target_link_libraries(GravitySimulation sfml-graphics sfml-audio)  # Link SFML to your project

include_directories(src/headers)
