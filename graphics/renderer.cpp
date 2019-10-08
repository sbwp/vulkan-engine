//
// Created by sabrina on 10/1/19.
//

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>
#include <algorithm>

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"

namespace Graphics {
    Renderer::Renderer(Core::Game& game):  game(game), window(1820, 980, "Vulkan Engine") {
        glfw::appendRequiredExtensions(instanceExtensions);
        setupValidationLayers(instanceExtensions, validationLayers);
        createInstance();
        surface = window.createSurface(instance);
        choosePhysicalDevice();
        createLogicalDevice();
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
        auto vulkanPhysicalDevices = instance.enumeratePhysicalDevices();
        Logger::assertNotEmpty(vulkanPhysicalDevices, "No physical devices found.");

        physicalDevices.reserve(vulkanPhysicalDevices.size());
        for (auto& device : vulkanPhysicalDevices) {
            physicalDevices.emplace_back(device, surface, deviceExtensions);
        }

        auto best = std::max_element(begin(physicalDevices), end(physicalDevices));
        Logger::assertTrue(best != end(physicalDevices) && best->isUsable(), "No suitable GPU found.");

        physicalDevice = &(*best);
    }

    void Renderer::createLogicalDevice() {
        float queuePriority = 1.0f;
        auto queueCreateInfos = physicalDevice->getDeviceQueueCreateInfos(&queuePriority);

        vk::PhysicalDeviceFeatures deviceFeatures{};
        // deviceFeatures.whatever = true; // Get rid of this comment once there is one to use as an example.

        vk::DeviceCreateInfo deviceCreateInfo{
            vk::DeviceCreateFlags(),
            static_cast<uint32_t>(queueCreateInfos.size()),
            queueCreateInfos.data(),
            static_cast<uint32_t>(validationLayers.size()),
            validationLayers.empty() ? nullptr : validationLayers.data(),
            static_cast<uint32_t>(deviceExtensions.size()),
            deviceExtensions.data(),
            &deviceFeatures
        };

        logicalDevice = physicalDevice->createLogicalDevice(deviceCreateInfo);

    }
}
