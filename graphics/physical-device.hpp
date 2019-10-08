//
// Created by sabrina on 10/7/19.
//

#ifndef VULKAN_ENGINE_PHYSICAL_DEVICE_HPP
#define VULKAN_ENGINE_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include <optional>

namespace Graphics {
    class PhysicalDevice {
    public:
        PhysicalDevice(vk::PhysicalDevice device, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions);

        bool operator<(PhysicalDevice& other);
        std::vector<vk::DeviceQueueCreateInfo> getDeviceQueueCreateInfos(float* queuePriorities);
        vk::Device createLogicalDevice(vk::DeviceCreateInfo const& createInfo);
        bool isUsable();
    private:
        vk::PhysicalDevice device;
        vk::SurfaceKHR& surface;

        std::vector<vk::QueueFamilyProperties> queueFamilies;
        std::vector<vk::ExtensionProperties> extensionProperties;
        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> presentModes;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        vk::PhysicalDeviceProperties properties;

        std::optional<uint32_t> graphicsQueueFamilyIndex;
        std::optional<uint32_t> presentQueueFamilyIndex;

        int rating;

        int rate(std::vector<char const*> const& deviceExtensions);
        std::optional<uint32_t> findGraphicsQueueFamilyIndex();
        std::optional<uint32_t> findPresentQueueFamilyIndex();
        static vk::DeviceQueueCreateInfo generateDeviceQueueCreateInfo(uint32_t index, float* queuePriorities);
    };
}

#endif //VULKAN_ENGINE_PHYSICAL_DEVICE_HPP
