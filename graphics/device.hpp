//
// Created by sabrina on 10/7/19.
//

#ifndef VULKAN_ENGINE_DEVICE_HPP
#define VULKAN_ENGINE_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include <vma.hpp>
#include <optional>
#include "image.hpp"

namespace Graphics {
	class Device {
	public:
		Device(vk::PhysicalDevice device, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions);

		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		std::vector<vk::PresentModeKHR> presentModes;
		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		bool operator<(Device& other);
		std::vector<vk::DeviceQueueCreateInfo> getDeviceQueueCreateInfos(float* queuePriorities);
		bool isUsable();

		uint32_t graphicsIndex();
		uint32_t presentIndex();

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		vk::Semaphore createSemaphore(vk::SemaphoreCreateInfo const& info);
		vk::Fence createFence(vk::FenceCreateInfo const& info);
		vk::CommandPool createCommandPool(vk::CommandPoolCreateInfo const& info);
		std::vector<vk::CommandBuffer> allocateCommandBuffers(vk::CommandBufferAllocateInfo const& info);
		vk::SwapchainKHR createSwapchain(vk::SwapchainCreateInfoKHR const& info);

		std::vector<Image> getSwapchainImages(vk::SwapchainKHR const& swapchain, vk::Format const& format,
												  vk::ImageView const& depthImageView);

		vk::ImageView createImageView(vk::ImageViewCreateInfo const& info);
		vk::RenderPass createRenderPass(vk::RenderPassCreateInfo const& info);
		vk::FormatProperties getFormatProperties(vk::Format const& format);
		vk::FormatFeatureFlags getFormatFeaturesForTiling(vk::Format const& format, vk::ImageTiling const& tiling);

		vk::Image createImage(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
					vk::ImageUsageFlagBits usageFlags,
					vk::MemoryPropertyFlagBits memoryPropertyFlags, uint32_t mipLevels);

		std::vector<vk::Framebuffer> createFramebuffers(vk::FramebufferCreateInfo createInfo,
														std::vector<Image> const& images);

		void createLogicalDevice(std::vector<char const*> const& deviceExtensions,
								 std::vector<char const*> const& validationLayers);

		vma::Allocator createAllocator();
	private:
		vk::PhysicalDevice physicalDevice;
		vk::Device logicalDevice;
		vk::SurfaceKHR& surface;

		std::vector<vk::QueueFamilyProperties> queueFamilies;
		std::vector<vk::ExtensionProperties> extensionProperties;

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

#endif //VULKAN_ENGINE_DEVICE_HPP
