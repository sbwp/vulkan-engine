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
        static int const maxFrames = 2;
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

        std::vector<vk::Semaphore> semaphores;
        std::vector<vk::Fence> commandBufferFences;

        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        vk::SurfaceFormatKHR surfaceFormat;
        vk::PresentModeKHR presentMode;
        vk::Extent2D extent;
        vk::SwapchainKHR swapchain;

        void run() override;
        bool shouldContinue() override;

        void createInstance();
        void choosePhysicalDevice();
        void createLogicalDevice();
        void createSynchronization();
        void createCommandPool();
        void createCommandBuffers();
        void createSwapchain();

        vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR>& supportedFormats);

        vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR>& supportedModes);

        vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities);
    };
}

#endif //VULKAN_ENGINE_RENDERER_HPP
