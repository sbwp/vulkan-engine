//
// Created by sabrina on 10/1/19.
//

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>
#include <algorithm>

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"
#include "../util/algorithm.hpp"

namespace Graphics {
	Renderer::Renderer(Core::Game& game) : game(game), window(1820, 980, "Vulkan Engine"),
										   presentMode(vk::PresentModeKHR::eFifo), depthFormat(vk::Format::eUndefined) {
		glfw::appendRequiredExtensions(instanceExtensions);
		setupValidationLayers(instanceExtensions, validationLayers);
		createInstance();
		surface = window.createSurface(instance);
		choosePhysicalDevice();
		device->createLogicalDevice(deviceExtensions, validationLayers);
		allocator = device->createAllocator();
		createSynchronization();
		createCommandPool();
		createCommandBuffers();
		surfaceFormat = chooseSurfaceFormat(device->surfaceFormats);
		presentMode = choosePresentMode(device->presentModes);
		extent = chooseExtent(device->surfaceCapabilities);
		createDepthImage();
		createSwapchain();
		createRenderPass();
		createFramebuffers();
	}

	Renderer::~Renderer() = default; // Remove if nothing ends up going here.

	void Renderer::run() {
		glfw::tick();
	}

	bool Renderer::shouldContinue() {
		return !window.shouldClose();
	}

	void Renderer::createInstance() {
		auto appInfo = game.makeAppInfo();
		vk::InstanceCreateInfo createInfo{
			{},
			&appInfo,
			static_cast<uint32_t>(validationLayers.size()),
			validationLayers.data(),
			static_cast<uint32_t>(instanceExtensions.size()),
			instanceExtensions.data()
		};
		instance = vk::createInstance(createInfo);

		setupValidationCallback();
	}

	void Renderer::choosePhysicalDevice() {
		auto physicalDevices = instance.enumeratePhysicalDevices();
		Logger::assertNotEmpty(physicalDevices, "No physical devices found.");

		devices.reserve(physicalDevices.size());
		for (auto& physicalDevice : physicalDevices) {
			devices.emplace_back(physicalDevice, surface, deviceExtensions);
		}

		auto best = std::max_element(begin(devices), end(devices));
		Logger::assertTrue(best != end(devices) && best->isUsable(), "No suitable GPU found.");

		device = &(*best);
	}

	void Renderer::createSynchronization() {
		vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		vk::FenceCreateInfo fenceCreateInfo{};
		semaphores.reserve(maxFrames);
		commandBufferFences.reserve(maxFrames);
		for (int i = 0; i < maxFrames; ++i) {
			semaphores.push_back(device->createSemaphore(semaphoreCreateInfo));
			commandBufferFences.push_back(device->createFence(fenceCreateInfo));
		}
	}

	void Renderer::createCommandPool() {
		commandPool = device->createCommandPool({
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			device->graphicsIndex()
		});

	}

	void Renderer::createCommandBuffers() {
		commandBuffers = device->allocateCommandBuffers({
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			maxFrames
		});
	}

	void Renderer::createSwapchain() {
		uint32_t indices[] = {device->graphicsIndex(), device->presentIndex()};
		bool queuesSame = indices[0] == indices[1];

		swapchain = device->createSwapchain({
			{},
			surface,
			maxFrames,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			extent,
			1u,
			vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eTransferSrc,
			queuesSame
			? vk::SharingMode::eExclusive
			: vk::SharingMode::eConcurrent,
			queuesSame
			? 1u
			: 2u,
			indices,
			vk::SurfaceTransformFlagBitsKHR::eIdentity,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode
		});

		images = device->getSwapchainImages(swapchain, surfaceFormat.format, depthImageView);
	}

	vk::SurfaceFormatKHR Renderer::chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR>& supportedFormats) {
		if (supportedFormats.size() == 1 && supportedFormats[0].format == vk::Format::eUndefined) {
			return {
				vk::Format::eB8G8R8A8Unorm,
				vk::ColorSpaceKHR::eSrgbNonlinear
			};
		}

		for (const auto& format : supportedFormats) {
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return format;
			}
		}

		return supportedFormats[0];
	}

	vk::PresentModeKHR Renderer::choosePresentMode(std::vector<vk::PresentModeKHR>& supportedModes) {
		for (const auto& mode : supportedModes) {
			if (mode == vk::PresentModeKHR::eMailbox) {
				return mode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D Renderer::chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}

		auto windowExtent = window.getFramebufferSize();
		windowExtent.width = Util::clamp(windowExtent.width, capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		windowExtent.height = Util::clamp(windowExtent.height, capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
		return windowExtent;
	}

	void Renderer::createDepthImage() {
		std::vector<vk::Format> formats{
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint
		};

		depthFormat = chooseSupportedFormat(formats, vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);

		depthImage = device->createImage(depthFormat, extent, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, mipLevels);

		depthImageView = device->createImageView({
			{},
			depthImage,
			vk::ImageViewType::e2D,
			depthFormat,
			{},
			{
				vk::ImageAspectFlagBits::eDepth,
				0u,
				mipLevels,
				0u,
				1u
			}
		});
	}

	vk::Format Renderer::chooseSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling,
											   const vk::FormatFeatureFlags& features) {
		for (const auto& format : formats) {
			auto supportedFeatures = device->getFormatFeaturesForTiling(format, tiling);
			if ((supportedFeatures & features) == features) {
				return format;
			}
		}

		throw Logger::error{"No supported format found."};
	}

	void Renderer::createRenderPass() {
		std::vector<vk::AttachmentDescription> attachments;

		// Color Attachment
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags{},
			surfaceFormat.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		);

		// Depth Attachment
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags{},
			depthFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eLoad,
			vk::AttachmentStoreOp::eStore,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		);

		vk::AttachmentReference colorReference{
			0u,
			vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::AttachmentReference depthReference{
			1u,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		vk::SubpassDescription subpassDescription{
			{},
			vk::PipelineBindPoint::eGraphics,
			0u,
			nullptr,
			1u,
			&colorReference,
			nullptr,
			&depthReference
		};

		renderPass = device->createRenderPass({
			{},
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			1u,
			&subpassDescription,
			0u,
			nullptr
		});
	}

	void Renderer::createFramebuffers() {
		vk::ImageView attachments[2];
		attachments[1] = depthImageView;
		vk::Extent2D windowExtent = window.getFramebufferSize();

		framebuffers = device->createFramebuffers({
			{},
			renderPass,
			2u,
			nullptr,
			windowExtent.width,
			windowExtent.height,
			1u
		}, images);
	}
}
