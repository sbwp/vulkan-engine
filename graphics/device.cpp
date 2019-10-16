//
// Created by sabrina on 10/7/19.
//

#include "device.hpp"
#include "../logger/logger.hpp"
#include "../util/algorithm.hpp"
#include "image.hpp"

namespace Graphics {
	Device::Device(vk::PhysicalDevice device, vk::SurfaceKHR& surface,
				   std::vector<char const*> const& deviceExtensions,
				   std::vector<char const*> const& validationLayers) : physicalDevice(device), surface(surface) {
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

		createLogicalDevice(deviceExtensions, validationLayers);
	}

	std::vector<vk::DeviceQueueCreateInfo> Device::getDeviceQueueCreateInfos(float* queuePriorities) {
		uint32_t presentQFIndex = Logger::unwrap(presentQueueFamilyIndex,
			"Physical device does not have present queue.");
		uint32_t graphicsQFIndex = Logger::unwrap(graphicsQueueFamilyIndex,
			"Physical device does not have graphics queue.");
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

		queueCreateInfos.emplace_back(generateDeviceQueueCreateInfo(graphicsQFIndex, queuePriorities));

		if (presentQFIndex != graphicsQFIndex) {
			queueCreateInfos.emplace_back(generateDeviceQueueCreateInfo(presentQFIndex, queuePriorities));
		}

		return queueCreateInfos;
	}

	int Device::rate(std::vector<char const*> const& deviceExtensions) {
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

	bool Device::operator<(Device& other) {
		return rating < other.rating;
	}

	std::optional<uint32_t> Device::findGraphicsQueueFamilyIndex() {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
			vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
			if (queueFamilyProperties.queueCount > 0 &&
				queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
				return i;
			}
		}
		return {};
	}

	std::optional<uint32_t> Device::findPresentQueueFamilyIndex() {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i) {
			vk::QueueFamilyProperties& queueFamilyProperties = queueFamilies[i];
			if (queueFamilyProperties.queueCount > 0 &&
				physicalDevice.getSurfaceSupportKHR(i, surface)) {
				return i;
			}
		}
		return {};
	}

	vk::DeviceQueueCreateInfo Device::generateDeviceQueueCreateInfo(uint32_t index, float* queuePriorities) {
		return vk::DeviceQueueCreateInfo{
			vk::DeviceQueueCreateFlags(),
			index,
			1u,
			queuePriorities
		};
	}

	void Device::createLogicalDevice(std::vector<char const*> const& deviceExtensions,
									 std::vector<char const*> const& validationLayers) {
		float queuePriority = 1.0f;
		auto queueCreateInfos = getDeviceQueueCreateInfos(&queuePriority);

		vk::PhysicalDeviceFeatures deviceFeatures{};
		// deviceFeatures.whatever = true; // Get rid of this comment once there is one to use as an example.

		logicalDevice = physicalDevice.createDevice({
			vk::DeviceCreateFlags(),
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			static_cast<uint32_t>(validationLayers.size()),
			validationLayers.empty()
			? nullptr
			: validationLayers.data(),
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data(),
			&deviceFeatures
		});
		graphicsQueue = logicalDevice.getQueue(graphicsIndex(), 0);
		presentQueue = logicalDevice.getQueue(presentIndex(), 0);
	}

	bool Device::isUsable() {
		return rating > 0;
	}

	uint32_t Device::graphicsIndex() {
		return Logger::unwrap(graphicsQueueFamilyIndex, "Device does not have graphics queue.");
	}

	uint32_t Device::presentIndex() {
		return Logger::unwrap(presentQueueFamilyIndex, "Device does not have present queue.");
	}

	vk::Semaphore Device::createSemaphore(vk::SemaphoreCreateInfo const& info) {
		return logicalDevice.createSemaphore(info);
	}

	vk::Fence Device::createFence(vk::FenceCreateInfo const& info) {
		return logicalDevice.createFence(info);
	}

	vk::CommandPool Device::createCommandPool(vk::CommandPoolCreateInfo const& info) {
		return logicalDevice.createCommandPool(info);
	}

	std::vector<vk::CommandBuffer> Device::allocateCommandBuffers(vk::CommandBufferAllocateInfo const& info) {
		return logicalDevice.allocateCommandBuffers(info);
	}

	vk::SwapchainKHR Device::createSwapchain(vk::SwapchainCreateInfoKHR const& info) {
		return logicalDevice.createSwapchainKHR(info);
	}

	std::vector<Image> Device::getSwapchainImages(vk::SwapchainKHR const& swapchain, vk::Format const& format,
												  vk::ImageView const& depthImageView) {
		auto swapchainImages = logicalDevice.getSwapchainImagesKHR(swapchain);
		std::vector<Image> images{};
		images.reserve(swapchainImages.size());

		for (auto& image : swapchainImages) {
			images.emplace_back(image, createImageView({
				vk::ImageViewCreateFlags(),
				image,
				vk::ImageViewType::e2D,
				format,
				vk::ComponentMapping{
					vk::ComponentSwizzle::eR,
					vk::ComponentSwizzle::eG,
					vk::ComponentSwizzle::eB,
					vk::ComponentSwizzle::eA
				},
				vk::ImageSubresourceRange{
					vk::ImageAspectFlagBits::eColor,
					0u,
					1u,
					0u,
					1u
				}
			}), depthImageView);
		}

		return images;
	}

	vk::ImageView Device::createImageView(vk::ImageViewCreateInfo const& info) {
		return logicalDevice.createImageView(info);
	}

	vk::FormatProperties Device::getFormatProperties(vk::Format const& format) {
		return physicalDevice.getFormatProperties(format);
	}

	vk::FormatFeatureFlags Device::getFormatFeaturesForTiling(vk::Format const& format, vk::ImageTiling const& tiling) {
		auto formatProperties = getFormatProperties(format);
		if (tiling == vk::ImageTiling::eLinear) {
			return formatProperties.linearTilingFeatures;
		}
		if (tiling == vk::ImageTiling::eOptimal) {
			return formatProperties.optimalTilingFeatures;
		}
		return vk::FormatFeatureFlags();
	}

	vk::RenderPass Device::createRenderPass(vk::RenderPassCreateInfo const& info) {
		return logicalDevice.createRenderPass(info);
	}

	vk::Image Device::createImage(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
								  vk::ImageUsageFlagBits usageFlags,
								  vk::MemoryPropertyFlagBits memoryPropertyFlags, uint32_t mipLevels) {
		return logicalDevice.createImage({
			vk::ImageCreateFlags(),
			vk::ImageType::e2D,
			format,
			{extent.width, extent.height, 1u},
			mipLevels,
			1u,
			vk::SampleCountFlagBits::e1,
			tiling,
			usageFlags
		});
	}

	std::vector<vk::Framebuffer> Device::createFramebuffers(vk::FramebufferCreateInfo createInfo,
															std::vector<Image> const& images) {
		std::vector<vk::Framebuffer> framebuffers{};
		framebuffers.reserve(images.size());

		for (auto const& image : images) {
			createInfo.pAttachments = image.getAttachments();
			framebuffers.push_back(logicalDevice.createFramebuffer(createInfo));
		}

		return framebuffers;
	}
}
