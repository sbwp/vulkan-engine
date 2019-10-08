cmake_minimum_required(VERSION 3.15)
project(vulkan_engine)
find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(vulkan_engine main.cpp graphics/renderer.cpp graphics/renderer.hpp logger/logger.hpp graphics/validation.cpp graphics/validation.hpp graphics/vulkan-state.cpp graphics/vulkan-state.hpp core/game.hpp util/runnable.hpp util/runnable.cpp glfw/window.cpp glfw/window.hpp graphics/physical-device.cpp graphics/physical-device.hpp util/algorithm.hpp logger/logger.cpp)

target_link_libraries(vulkan_engine glfw)
target_link_libraries(vulkan_engine Vulkan::Vulkan)
