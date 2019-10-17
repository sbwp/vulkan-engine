cmake_minimum_required(VERSION 3.15)
project(vulkan_engine)
find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(includes)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_definitions(DEBUG)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_executable(vulkan_engine main.cpp graphics/renderer.cpp graphics/renderer.hpp logger/logger.hpp graphics/validation.cpp graphics/validation.hpp graphics/vulkan-state.cpp graphics/vulkan-state.hpp core/game.hpp util/runnable.hpp util/runnable.cpp glfw/window.cpp glfw/window.hpp graphics/device.cpp graphics/device.hpp util/algorithm.hpp logger/logger.cpp graphics/image.cpp graphics/image.hpp graphics/vma-impl.cpp includes/vma.hpp)

target_link_libraries(vulkan_engine glfw)
target_link_libraries(vulkan_engine Vulkan::Vulkan)
