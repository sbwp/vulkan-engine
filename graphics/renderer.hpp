//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_RENDERER_HPP
#define VULKAN_ENGINE_RENDERER_HPP

#include <cstdint>
#include <vector>
#include <string>

#include "../glfw/window.hpp"
#include "../core/game.hpp"
#include "../util/runnable.hpp"
#include "physical-device.hpp"

namespace Graphics {
    class Renderer: public Util::Runnable {
    public:
        explicit Renderer(Core::Game& game);
        ~Renderer() override;

    private:
        Core::Game& game;

        std::vector<const char*> instanceExtensions {
            VK_KHR_SURFACE_EXTENSION_NAME
        };
        const std::vector<const char*> deviceExtensions {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        std::vector<const char*> validationLayers {};
        glfw::Window window;
        vk::Instance instance;
        vk::SurfaceKHR surface;
        std::vector<PhysicalDevice> physicalDevices;
        PhysicalDevice* physicalDevice = nullptr;
        vk::Device logicalDevice;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        void run() override;
        bool shouldContinue() override;

        void createInstance();
        void choosePhysicalDevice();
        void createLogicalDevice();
    };
}

#endif //VULKAN_ENGINE_RENDERER_HPP
