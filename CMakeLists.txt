cmake_minimum_required(VERSION 3.15)
project(vulkan_engine)

set(CMAKE_CXX_STANDARD 17)

add_executable(vulkan_engine main.cpp graphics/renderer.cpp graphics/renderer.hpp logger/logger.hpp graphics/validation.cpp graphics/validation.hpp graphics/vulkan-state.cpp graphics/vulkan-state.hpp core/game.hpp)
