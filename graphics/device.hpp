//
// Created by sabrina on 10/7/19.
//

#ifndef VULKAN_ENGINE_DEVICE_HPP
#define VULKAN_ENGINE_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include <optional>

namespace Graphics {
    class Device {
    public:
        Device(vk::PhysicalDevice device, vk::SurfaceKHR &surface, std::vector<char const *> const &deviceExtensions,
               std::vector<char const *> const &validationLayers);

        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> presentModes;
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;

        bool operator<(Device &other);

        std::vector<vk::DeviceQueueCreateInfo> getDeviceQueueCreateInfos(float *queuePriorities);

        bool isUsable();

        uint32_t graphicsIndex();

        uint32_t presentIndex();

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vk::Semaphore createSemaphore(vk::SemaphoreCreateInfo info);
        vk::Fence createFence(vk::FenceCreateInfo info);
        vk::CommandPool createCommandPool(vk::CommandPoolCreateInfo info);
        std::vector<vk::CommandBuffer> allocateCommandBuffers(vk::CommandBufferAllocateInfo info);
        vk::SwapchainKHR createSwapchain(vk::SwapchainCreateInfoKHR info);
        std::vector<vk::Image> getSwapchainImages(vk::SwapchainKHR swapchain);
        vk::ImageView createImageView(vk::ImageViewCreateInfo info);

    private:
        vk::PhysicalDevice physicalDevice;
        vk::Device logicalDevice;
        vk::SurfaceKHR &surface;

        std::vector<vk::QueueFamilyProperties> queueFamilies;
        std::vector<vk::ExtensionProperties> extensionProperties;

        vk::PhysicalDeviceMemoryProperties memoryProperties;
        vk::PhysicalDeviceProperties properties;

        std::optional<uint32_t> graphicsQueueFamilyIndex;
        std::optional<uint32_t> presentQueueFamilyIndex;

        int rating;

        int rate(std::vector<char const *> const &deviceExtensions);

        std::optional<uint32_t> findGraphicsQueueFamilyIndex();

        std::optional<uint32_t> findPresentQueueFamilyIndex();

        static vk::DeviceQueueCreateInfo generateDeviceQueueCreateInfo(uint32_t index, float *queuePriorities);

        void createLogicalDevice(std::vector<char const *> const &deviceExtensions,
                                 std::vector<char const *> const &validationLayers);
    };
}

#endif //VULKAN_ENGINE_DEVICE_HPP
