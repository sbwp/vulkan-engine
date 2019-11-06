//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_RENDERER_HPP
#define VULKAN_ENGINE_RENDERER_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <vulkan/vulkan.hpp>
#include <vma.hpp>

#include "../glfw/window.hpp"
#include "../core/game.hpp"
#include "../util/runnable.hpp"
#include "device.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "buffer.hpp"
#include "uniform-buffer-object.hpp"

namespace Graphics {
	class Renderer: public Util::Runnable {
	public:
		explicit Renderer(Core::Game& game);

	private:
		static int const maxFrames = 2;
		int currentFrame = 0;
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

		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderFinishedSemaphores;
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
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::Pipeline graphicsPipeline;
		Image textureImage;
		vk::Sampler textureSampler;

		Buffer vertexBuffer;
		Buffer indexBuffer;
		std::vector<Buffer> uniformBuffers;

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};
		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
		UniformBufferObject ubo;

		const uint32_t mipLevels = 1u; // TODO - actually implement miplevels

		static vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& supportedFormats);
		static vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR> const& supportedModes);

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
		void createTextureImage();
		void createGraphicsPipeline();
		void destroySwapchainAndFriends();
		void recreateSwapchain();
		void createSwapchainAndFriends();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();
		void createTextureSampler();

		vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR const& capabilities);

		vk::Format chooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling,
										 const vk::FormatFeatureFlags& features);
		void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);
		void updateUniformBuffer(uint32_t index);
		void copyMemory(vma::Allocation const& allocation, void* data, size_t size);
		void runCommand(std::function<void(vk::CommandBuffer)> const& callback);
		void transitionImageLayout(Image& image, vk::Format const& format, vk::ImageLayout const& from, vk::ImageLayout const& to);
		void copyBufferToImage(Buffer& buffer, Image& image, uint32_t width, uint32_t height);
		static vk::AccessFlags accessMaskForLayout(vk::ImageLayout const& layout);
		static vk::PipelineStageFlags pipelineStageForLayout(vk::ImageLayout const& layout);
	};
}

#endif //VULKAN_ENGINE_RENDERER_HPP
