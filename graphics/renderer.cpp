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
    Renderer::Renderer(Core::Game game) {
        initializeGLFW();
        setupValidationLayers(instanceExtensions, validationLayers);

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

    Renderer::~Renderer() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Renderer::initializeGLFW() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1820, 980, "Vulkan Engine", nullptr, nullptr);
    }

    void Renderer::run() {
        glfwPollEvents();
    }

    bool Renderer::shouldContinue() {
        return !glfwWindowShouldClose(window);
    }
}
