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
        window = glfw::Window(1820, 980, "Vulkan Engine");
        glfw::appendRequiredExtensions(instanceExtensions);
        setupValidationLayers(instanceExtensions, validationLayers);
        createInstance();
        surface = window.createSurface(instance);
        choosePhysicalDevice();
    }

    Renderer::~Renderer() = default; // Remove if nothing ends up going here.

    void Renderer::run() {
        glfw::tick();
    }

    bool Renderer::shouldContinue() {
        return !window.shouldClose();
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

    void Renderer::choosePhysicalDevice() {
        auto devices = instance.enumeratePhysicalDevices();
        Logger::assertNotEmpty(devices, "No physical devices found.");
        for (auto device : devices) {
            auto queueFamilies = device.getQueueFamilyProperties();
            Logger::assertNotEmpty(queueFamilies, "No Queue families found.");
        }
    }
}
