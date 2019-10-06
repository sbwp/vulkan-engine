//
// Created by sabrina on 10/1/19.
//

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"

namespace Graphics {
    Renderer::Renderer(Core::Game& game): game(game) {
        initializeGLFW();
        setupValidationLayers(instanceExtensions, validationLayers);
        createInstance();
        createSurface();

    }

    Renderer::~Renderer() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Renderer::initializeGLFW() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1820, 980, "Vulkan Engine", nullptr, nullptr);
        uint32_t glfwRequiredExtensionsCount;
        auto glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
        for (uint32_t i = 0; i < glfwRequiredExtensionsCount; ++i) {
            instanceExtensions.push_back(glfwRequiredExtensions[i]);
        }
    }

    void Renderer::run() {
        glfwPollEvents();
    }

    bool Renderer::shouldContinue() {
        return !glfwWindowShouldClose(window);
    }

    void Renderer::createInstance() {
        auto appInfo = game.makeAppInfo();
        vk::InstanceCreateInfo createInfo {
                vk::InstanceCreateFlags(),
                &appInfo,
                static_cast<uint32_t>(validationLayers.size()),
                validationLayers.data(),
                static_cast<uint32_t>(instanceExtensions.size()),
                instanceExtensions.data()
        };
        instance = vk::createInstance(createInfo);

        setupValidationCallback();
    }

    void Renderer::createSurface() {
        VkSurfaceKHR c_surface;
        if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &c_surface) != VK_SUCCESS) {
            throw Logger::error("failed to create window surface!");
        }
        surface = vk::SurfaceKHR(c_surface);
    }
}
