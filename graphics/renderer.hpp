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
#include "device.hpp"

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

        std::vector<Device> physicalDevices;
        Device* device = nullptr;

        std::vector<vk::Semaphore> semaphores;
        std::vector<vk::Fence> commandBufferFences;

        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        vk::SurfaceFormatKHR surfaceFormat{};
        vk::PresentModeKHR presentMode;
        vk::Extent2D extent;
        vk::SwapchainKHR swapchain;
        vk::Format depthFormat;
        vk::RenderPass renderPass;


        static vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR>& supportedFormats);
        static vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR>& supportedModes);

        void run() override;
        bool shouldContinue() override;

        void createInstance();
        void choosePhysicalDevice();
        void createSynchronization();
        void createCommandPool();
        void createCommandBuffers();
        void createSwapchain();
        void createRenderTargets();
        void createRenderPass();

        vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities);
        vk::Format chooseSupportedFormat(std::vector<vk::Format> formats, vk::ImageTiling tiling,
                vk::FormatFeatureFlags features);
    };
}

#endif //VULKAN_ENGINE_RENDERER_HPP
