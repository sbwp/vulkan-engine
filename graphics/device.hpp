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
		Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR& surface, std::vector<char const*> const& deviceExtensions);

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

		std::vector<Image> getSwapchainImages(vk::SwapchainKHR const& swapchain, vk::Format const& format);

		vk::ImageView createImageView(vk::ImageViewCreateInfo const& info);
		vk::RenderPass createRenderPass(vk::RenderPassCreateInfo const& info);
		vk::FormatProperties getFormatProperties(vk::Format const& format);
		vk::FormatFeatureFlags getFormatFeaturesForTiling(vk::Format const& format, vk::ImageTiling const& tiling);

		vk::Image createImage(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
					vk::ImageUsageFlagBits usageFlags,
					vk::MemoryPropertyFlagBits memoryPropertyFlags, uint32_t mipLevels);

		std::vector<vk::Framebuffer> createFramebuffers(vk::FramebufferCreateInfo createInfo,
														std::vector<Image> const& images,
														vk::ImageView const& depthView);

		void createLogicalDevice(std::vector<char const*> const& deviceExtensions,
								 std::vector<char const*> const& validationLayers);

		vma::Allocator createAllocator();
		vk::MemoryRequirements getImageMemoryRequirements(vk::Image image);
		vk::ShaderModule createShaderModule(std::vector<char> code);
		vk::PipelineLayout createPipelineLayout(vk::PipelineLayoutCreateInfo info);
		vk::Pipeline createGraphicsPipeline(vk::GraphicsPipelineCreateInfo info);
		vk::ResultValue<uint32_t> acquireNextImage(vk::SwapchainKHR swapchain, vk::Semaphore semaphore);
		void resetFence(vk::Fence fence);
		void waitForFence(vk::Fence& fence);
		void waitUntilIdle();
		void printDescription();
		void destroySwapchain(vk::SwapchainKHR swapchain, std::vector<vk::Framebuffer> framebuffers, vk::CommandPool commandPool,
							  std::vector<vk::CommandBuffer> commandBuffers, vk::Pipeline pipeline, vk::PipelineLayout layout,
							  vk::RenderPass renderPass, std::vector<Image> images);
		vk::SurfaceCapabilitiesKHR getCapabilities();
		std::vector<vk::SurfaceFormatKHR> getSurfaceFormats();
		std::vector<vk::PresentModeKHR> getPresentModes();
		vk::DescriptorSetLayout createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const& createInfo);
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
