//
// Created by sabrina on 10/7/19.
//

#include "physical-device.hpp"
#include "../logger/logger.hpp"
#include "../util/algorithm.hpp"

namespace Graphics {
    PhysicalDevice::PhysicalDevice(vk::PhysicalDevice &device, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions): device(device), surface(surface) {
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
        rating = rate(deviceExtensions);
        graphicsQueueFamilyIndex = getGraphicsQueueFamilyIndex();
        presentQueueFamilyIndex = getPresentQueueFamilyIndex();
    }

    int PhysicalDevice::rate(std::vector<char const*> const& deviceExtensions) {
        if (!(presentQueueFamilyIndex.has_value() && graphicsQueueFamilyIndex.has_value())) {
            return 0;
        }

        for (auto requiredExtension : deviceExtensions) {
            std::function<bool(vk::ExtensionProperties)> matchesRequirement =
                    [requiredExtension](vk::ExtensionProperties extension) -> bool {
                        return extension.extensionName == requiredExtension;
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

    std::optional<uint32_t> PhysicalDevice::getGraphicsQueueFamilyIndex() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
            vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
            if (queueFamilyProperties.queueCount > 0 &&
                queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
                return i;
            }
        }
        return {};
    }

    std::optional<uint32_t> PhysicalDevice::getPresentQueueFamilyIndex() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
            vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
            if (queueFamilyProperties.queueCount > 0 &&
                device.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
        }
        return {};
    }
}
