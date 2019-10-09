//
// Created by sabrina on 10/7/19.
//

#include "physical-device.hpp"
#include "../logger/logger.hpp"
#include "../util/algorithm.hpp"

namespace Graphics {
    PhysicalDevice::PhysicalDevice(vk::PhysicalDevice device, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions): device(device), surface(surface) {
        queueFamilies = device.getQueueFamilyProperties();
        Logger::assertNotEmpty(queueFamilies, "Device supports no queue families.");

        extensionProperties = device.enumerateDeviceExtensionProperties();
        Logger::assertNotEmpty(extensionProperties, "Device supports no extensions.");

        surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);

        surfaceFormats = device.getSurfaceFormatsKHR(surface);
        Logger::assertNotEmpty(surfaceFormats, "Device supports no surface formats.");

        presentModes = device.getSurfacePresentModesKHR(surface);
        Logger::assertNotEmpty(presentModes, "Device supports no present modes.");

        memoryProperties = device.getMemoryProperties();
        properties = device.getProperties();
        graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex();
        presentQueueFamilyIndex = findPresentQueueFamilyIndex();
        rating = rate(deviceExtensions);
    }

    std::vector<vk::DeviceQueueCreateInfo> PhysicalDevice::getDeviceQueueCreateInfos(float *queuePriorities) {
        uint32_t presentQFIndex = Logger::unwrap(presentQueueFamilyIndex, "Physical device does not have present queue.");
        uint32_t graphicsQFIndex = Logger::unwrap(graphicsQueueFamilyIndex, "Physical device does not have graphics queue.");
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

        queueCreateInfos.emplace_back(generateDeviceQueueCreateInfo(graphicsQFIndex, queuePriorities));

        if (presentQFIndex != graphicsQFIndex) {
            queueCreateInfos.emplace_back(generateDeviceQueueCreateInfo(presentQFIndex, queuePriorities));
        }

        return queueCreateInfos;
    }

    int PhysicalDevice::rate(std::vector<char const*> const& deviceExtensions) {
        if (!(presentQueueFamilyIndex.has_value() && graphicsQueueFamilyIndex.has_value())) {
            return 0;
        }

        for (auto requiredExtension : deviceExtensions) {
            std::function<bool(vk::ExtensionProperties)> matchesRequirement =
                    [requiredExtension](vk::ExtensionProperties extension) -> bool {
                        return strcmp(extension.extensionName, requiredExtension) == 0;
                    };
            if (!Util::containsWhere(extensionProperties, matchesRequirement)) {
                return 0;
            }
        }

        return 1;
    }

    bool PhysicalDevice::operator<(PhysicalDevice &other) {
        return rating < other.rating;
    }

    std::optional<uint32_t> PhysicalDevice::findGraphicsQueueFamilyIndex() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
            vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
            if (queueFamilyProperties.queueCount > 0 &&
                queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
                return i;
            }
        }
        return {};
    }

    std::optional<uint32_t> PhysicalDevice::findPresentQueueFamilyIndex() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
            vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
            if (queueFamilyProperties.queueCount > 0 &&
                device.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
        }
        return {};
    }

    vk::DeviceQueueCreateInfo PhysicalDevice::generateDeviceQueueCreateInfo(uint32_t index, float* queuePriorities) {
        return vk::DeviceQueueCreateInfo{
            vk::DeviceQueueCreateFlags(),
            index,
            1u,
            queuePriorities
        };
    }

    vk::Device PhysicalDevice::createLogicalDevice(vk::DeviceCreateInfo const& createInfo) {
        return device.createDevice(createInfo);
    }

    bool PhysicalDevice::isUsable() {
        return rating > 0;
    }

    uint32_t PhysicalDevice::graphicsIndex() {
        return Logger::unwrap(graphicsQueueFamilyIndex, "Device does not have graphics queue.");
    }

    uint32_t PhysicalDevice::presentIndex() {
        return Logger::unwrap(presentQueueFamilyIndex, "Device does not have present queue.");
    }
}
