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
        PhysicalDevice(vk::PhysicalDevice &device, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions);

        bool operator<(PhysicalDevice& other);
    private:
        vk::PhysicalDevice& device;
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
        std::optional<uint32_t> getGraphicsQueueFamilyIndex();
        std::optional<uint32_t> getPresentQueueFamilyIndex();
    };
}

#endif //VULKAN_ENGINE_PHYSICAL_DEVICE_HPP
