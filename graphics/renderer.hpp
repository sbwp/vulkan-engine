//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_RENDERER_HPP
#define VULKAN_ENGINE_RENDERER_HPP

#include <cstdint>
#include <vector>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../core/game.hpp"
#include "../util/runnable.hpp"

namespace Graphics {
class Renderer: public Util::Runnable {
    public:
        explicit Renderer(Core::Game game);
        ~Renderer() override;

    private:
        std::vector<const char*> instanceExtensions {
            VK_KHR_SURFACE_EXTENSION_NAME
        };
        const std::vector<const char*> deviceExtensions {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        std::vector<const char*> validationLayers {};
        GLFWwindow* window;
        vk::Instance instance;

        void run() override;
        bool shouldContinue() override;

        void initializeGLFW();
    };
}

#endif //VULKAN_ENGINE_RENDERER_HPP
