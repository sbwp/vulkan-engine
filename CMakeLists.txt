cmake_minimum_required(VERSION 3.15)
project(vulkan_engine)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(includes)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/build")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # -O3")

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_definitions(DEBUG)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_custom_command(
        OUTPUT build/assets/shaders/fragment.spv
        DEPENDS graphics/shaders/shader.frag
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ./compile-shaders.sh
)

add_custom_command(
        OUTPUT build/assets/shaders/vertex.spv
        DEPENDS graphics/shaders/shader.vert
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ./compile-shaders.sh
)

add_custom_target(shaders ALL DEPENDS build/assets/shaders/fragment.spv build/assets/shaders/vertex.spv)

add_executable(vulkan_engine main.cpp graphics/renderer.cpp graphics/renderer.hpp logger/logger.hpp graphics/validation.cpp graphics/validation.hpp graphics/vulkan-state.cpp graphics/vulkan-state.hpp core/game.hpp util/runnable.hpp util/runnable.cpp glfw/window.cpp glfw/window.hpp graphics/device.cpp graphics/device.hpp util/algorithm.hpp logger/logger.cpp graphics/image.cpp graphics/image.hpp graphics/vma-impl.cpp includes/vma.hpp util/algorithm.cpp graphics/shader.cpp graphics/shader.hpp graphics/vertex.cpp graphics/vertex.hpp graphics/uniform-buffer-object.hpp graphics/buffer.cpp graphics/buffer.hpp)

add_dependencies(vulkan_engine shaders)

target_link_libraries(vulkan_engine glfw)
target_link_libraries(vulkan_engine Vulkan::Vulkan)
