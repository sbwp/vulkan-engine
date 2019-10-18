//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_RENDERER_HPP
#define VULKAN_ENGINE_RENDERER_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <vma.hpp>

#include "../glfw/window.hpp"
#include "../core/game.hpp"
#include "../util/runnable.hpp"
#include "device.hpp"
#include "image.hpp"

namespace Graphics {
	class Renderer: public Util::Runnable {
	public:
		explicit Renderer(Core::Game& game);

		~Renderer() override;

	private:
		static int const maxFrames = 2;
		Core::Game& game;

		std::vector<const char*> instanceExtensions{
			VK_KHR_SURFACE_EXTENSION_NAME
		};
		const std::vector<const char*> deviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		std::vector<const char*> validationLayers{};
		glfw::Window window;
		vk::Instance instance;
		vk::SurfaceKHR surface;

		std::vector<Device> devices;
		Device* device = nullptr;
		vma::Allocator allocator;

		std::vector<vk::Semaphore> semaphores;
		std::vector<vk::Fence> commandBufferFences;

		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;

		vk::SurfaceFormatKHR surfaceFormat{};
		vk::PresentModeKHR presentMode;
		vk::Extent2D extent;
		vk::SwapchainKHR swapchain;
		std::vector<Image> images;
		vk::Format depthFormat;
		Image depthImage;
		vk::RenderPass renderPass;
		std::vector<vk::Framebuffer> framebuffers;

		const uint32_t mipLevels = 1u; // TODO acutally implement miplevels

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
		void createDepthImage();
		void createRenderPass();
		void createFramebuffers();

		vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities);

		vk::Format chooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling,
										 const vk::FormatFeatureFlags& features);
		Image createImage(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
									vk::ImageUsageFlagBits usageFlags, vk::MemoryPropertyFlagBits memoryPropertyFlags,
									const vk::ImageAspectFlags& aspectFlags);
	};
}

#endif //VULKAN_ENGINE_RENDERER_HPP
