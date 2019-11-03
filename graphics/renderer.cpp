//
// Created by sabrina on 10/1/19.
//

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>
#include <algorithm>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"
#include "../util/algorithm.hpp"
#include "shader.hpp"
#include "uniform-buffer-object.hpp"

namespace Graphics {
	Renderer::Renderer(Core::Game& game): game(game), window(1820, 954, "Vulkan Engine"),
										  presentMode(vk::PresentModeKHR::eFifo), depthFormat(vk::Format::eUndefined) {
		glfw::appendRequiredExtensions(instanceExtensions);
		setupValidationLayers(instanceExtensions, validationLayers);
		createInstance();
		surface = window.createSurface(instance);
		choosePhysicalDevice();
		device->createLogicalDevice(deviceExtensions, validationLayers);
		allocator = device->createAllocator();
		createCommandPool();
		createSynchronization();

		createVertexBuffer();
		createIndexBuffer();
		createDescriptorSetLayout();
		createSwapchainAndFriends();
	}

	void Renderer::createSwapchainAndFriends() {
		surfaceFormat = chooseSurfaceFormat(device->getSurfaceFormats());
		presentMode = choosePresentMode(device->getPresentModes());
		extent = chooseExtent(device->getCapabilities());
		createSwapchain();
		createDepthImage();// TODO (Impl in progress)
		createRenderPass();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createGraphicsPipeline();
		createCommandBuffers();
	}

	void Renderer::recreateSwapchain() {
		device->waitUntilIdle();
		destroySwapchainAndFriends();
		createSwapchainAndFriends();
	}

	void Renderer::run() {
		glfw::tick();
		device->waitForFence(commandBufferFences[currentFrame]);
		vk::ResultValue<uint32_t> imageAcquisition(vk::Result::eSuccess, 0u);
		try {
			imageAcquisition = device->acquireNextImage(swapchain, imageAvailableSemaphores[currentFrame]);
		} catch (vk::OutOfDateKHRError& error) {
			recreateSwapchain();
			return;
		}
		if (imageAcquisition.result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapchain();
			return;
		} else if (imageAcquisition.result != vk::Result::eSuccess &&
			imageAcquisition.result != vk::Result::eSuboptimalKHR) {
			throw Logger::error("Swapchain image acquisition unsuccessful");
		}
		vk::PipelineStageFlags temp = vk::PipelineStageFlagBits::eVertexInput;
		updateUniformBuffer(imageAcquisition.value);
		vk::SubmitInfo submitInfo{
			1u,
			&(imageAvailableSemaphores[currentFrame]),
			&temp,
			1u,
			&(commandBuffers[imageAcquisition.value]),
			1u,
			&(renderFinishedSemaphores[currentFrame])
		};
		device->resetFence(commandBufferFences[currentFrame]);
		device->graphicsQueue.submit(1u, &submitInfo, commandBufferFences[currentFrame]);

		try {
			auto result = device->presentQueue.presentKHR({
				1,
				&(renderFinishedSemaphores[currentFrame]),
				1u,
				&swapchain,
				&(imageAcquisition.value),
				nullptr
			});

			if (window.shouldResize() || result == vk::Result::eSuboptimalKHR) {
				window.handleResize();
				throw vk::OutOfDateKHRError("Swapchain recreation required");
			}
		} catch (vk::OutOfDateKHRError& _) {
			recreateSwapchain();
		}

		currentFrame = (currentFrame + 1) % maxFrames;
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
		vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};

		imageAvailableSemaphores.reserve(maxFrames);
		renderFinishedSemaphores.reserve(maxFrames);
		commandBufferFences.reserve(maxFrames);
		for (int i = 0; i < maxFrames; ++i) {
			imageAvailableSemaphores.push_back(device->createSemaphore(semaphoreCreateInfo));
			renderFinishedSemaphores.push_back(device->createSemaphore(semaphoreCreateInfo));
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

		vk::CommandBufferBeginInfo beginInfo{};

		for (int i = 0; i < maxFrames; i++) {
			auto& commandBuffer = commandBuffers[i];
			commandBuffer.begin(&beginInfo);
			{
				auto clearColor = Util::makeClearColor(0.0f, 0.0f, 0.0f);
				vk::RenderPassBeginInfo renderPassBeginInfo{
					renderPass,
					framebuffers[i],
					{{0, 0}, extent},
					1u,
					&clearColor
				};

				commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
				{
					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

					vk::Buffer vertexBuffers[] = {vertexBuffer};
					vk::DeviceSize offsets[] = {0u};
					commandBuffer.bindVertexBuffers(0u, 1u, vertexBuffers, offsets);
					commandBuffer.bindIndexBuffer(indexBuffer, 0u, vk::IndexType::eUint32);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
						0u, 1u, &descriptorSets[i], 0u, nullptr);
					commandBuffer.drawIndexed(
						static_cast<uint32_t>(indices.size()),
						1u, 0u, 0u, 0u);
				}
				commandBuffer.endRenderPass();
			}
			commandBuffer.end();
		}
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
			device->getCapabilities().currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode
		});

		images = device->getSwapchainImages(swapchain, surfaceFormat.format);
	}

	vk::SurfaceFormatKHR Renderer::chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& supportedFormats) {
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

	vk::PresentModeKHR Renderer::choosePresentMode(std::vector<vk::PresentModeKHR> const& supportedModes) {
		for (const auto& mode : supportedModes) {
			if (mode == vk::PresentModeKHR::eMailbox) {
				return mode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D Renderer::chooseExtent(vk::SurfaceCapabilitiesKHR const& capabilities) {
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

		depthImage = createImage(depthFormat, extent, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth);
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
			vk::AttachmentLoadOp::eClear,
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

		vk::SubpassDependency dependency{
			VK_SUBPASS_EXTERNAL,
			0u,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlags(),
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
		};

		renderPass = device->createRenderPass({
			{},
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			1u,
			&subpassDescription,
			1u,
			&dependency
		});
	}

	void Renderer::createFramebuffers() {
		framebuffers = device->createFramebuffers({
			{},
			renderPass,
			2u,
			nullptr,
			extent.width,
			extent.height,
			1u
		}, images, depthImage.getView());
	}

	Image Renderer::createImage(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
								vk::ImageUsageFlagBits usageFlags, vk::MemoryPropertyFlagBits memoryPropertyFlags,
								const vk::ImageAspectFlags& aspectFlags) {
		auto imageAllocation = allocator.createImage({
			{},
			vk::ImageType::e2D,
			format,
			{extent.width, extent.height, 1u},
			mipLevels,
			1u,
			vk::SampleCountFlagBits::e1,
			tiling,
			usageFlags
		}, {
			{},
			vma::MemoryUsage::eGpuOnly
		});

		auto imageView = device->createImageView({
			{},
			imageAllocation.first,
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

		return Image(imageAllocation.first, imageView, imageAllocation.second);
	}

	void Renderer::createTextureImage() {

	}

	void Renderer::createGraphicsPipeline() {
		auto vertexShader = Shader("vertex", device, vk::ShaderStageFlagBits::eVertex);
		auto fragmentShader = Shader("fragment", device, vk::ShaderStageFlagBits::eFragment);

		vk::PipelineShaderStageCreateInfo shaderStages[] = {
			vertexShader.getShaderStageCreateInfo(),
			fragmentShader.getShaderStageCreateInfo()
		};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
			{},
			1u,
			&bindingDescription,
			static_cast<uint32_t>(attributeDescriptions.size()),
			attributeDescriptions.data()
		};

		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{
			{},
			vk::PrimitiveTopology::eTriangleList,
			false
		};

		vk::Viewport viewport{
			0.0f,
			0.0f,
			static_cast<float>(extent.width),
			static_cast<float>(extent.height),
			0.0f,
			1.0f
		};

		vk::Rect2D scissor = {{0, 0}, extent};

		vk::PipelineViewportStateCreateInfo viewportState{
			{},
			1,
			&viewport,
			1,
			&scissor
		};

		vk::PipelineRasterizationStateCreateInfo rasterizer{{},
			false,
			false,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise,
			false,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
			{},
			vk::SampleCountFlagBits::e1,
			false,
			1.0f
		};
		vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
			{},
			false, // TODO: Set to true to enable depth testing
			false, // TODO: Set to true to enable depth writing
			vk::CompareOp::eLess,
			false,
			false
		};

		vk::ColorComponentFlags rgbaColorComponents = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			false,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			rgbaColorComponents
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState{
			{},
			false,
			vk::LogicOp::eCopy,
			1u,
			&colorBlendAttachment
		};

		pipelineLayout = device->createPipelineLayout({
			{},
			1u,
			&descriptorSetLayout
			// TODO: Push constants
		});

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
			{},
			2u,
			shaderStages,
			&vertexInputStateCreateInfo,
			&pipelineInputAssemblyStateCreateInfo,
			nullptr,
			&viewportState,
			&rasterizer,
			&multisampleStateCreateInfo,
			&depthStencilStateCreateInfo,
			&colorBlendState,
			nullptr,
			pipelineLayout,
			renderPass,
			0u
		};

		graphicsPipeline = device->createGraphicsPipeline(pipelineCreateInfo);
	}

	void Renderer::destroySwapchainAndFriends() {
		device->destroySwapchain(swapchain, framebuffers, commandPool, commandBuffers, graphicsPipeline,
			renderPass, images, descriptorPool);
		for (auto& uniformBuffer : uniformBuffers) {
			allocator.freeMemory(uniformBuffer);
		}
		uniformBuffers.clear();
	}



	void Renderer::createVertexBuffer() {
		vk::DeviceSize const size = sizeof(vertices[0]) * vertices.size();
		Buffer stagingBuffer(allocator, size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			vma::MemoryUsage::eCpuToGpu);

		vertexBuffer = Buffer(allocator, size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vma::MemoryUsage::eGpuOnly);

		void* mappedData = allocator.mapMemory(stagingBuffer);
		memcpy(mappedData, vertices.data(), sizeof(vertices[0]) * vertices.size());
		allocator.unmapMemory(stagingBuffer);

		copyBuffer(stagingBuffer, vertexBuffer, size);
	}

	void Renderer::createIndexBuffer() {
		vk::DeviceSize size = sizeof(indices[0]) * indices.size();

		Buffer stagingBuffer(allocator, size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			vma::MemoryUsage::eCpuToGpu);

		void* mappedData = allocator.mapMemory(stagingBuffer);
		memcpy(mappedData, indices.data(), sizeof(indices[0]) * indices.size());
		allocator.unmapMemory(stagingBuffer);

		indexBuffer = Buffer(allocator, size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vma::MemoryUsage::eGpuOnly);

		copyBuffer(stagingBuffer, indexBuffer, size);
	}

	void Renderer::createUniformBuffers() {
		vk::DeviceSize size = sizeof(UniformBufferObject);

		uniformBuffers.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {
			uniformBuffers[i] = Buffer(allocator, size, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				vma::MemoryUsage::eCpuToGpu);
		}
	}

	void Renderer::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) {
		auto commandBuffer = device->allocateCommandBuffers({
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			1u
		})[0];
		commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
		{
			vk::BufferCopy copyRegion{0, 0, size};
			commandBuffer.copyBuffer(src, dst, 1u, &copyRegion);
		}
		commandBuffer.end();

		vk::SubmitInfo submitInfo{
			0u, nullptr, nullptr, 1u, &commandBuffer
		};
		device->graphicsQueue.submit(1u, &submitInfo, vk::Fence{});
	}

	void Renderer::createDescriptorSetLayout() {
		vk::DescriptorSetLayoutBinding binding{
			0u,
			vk::DescriptorType::eUniformBuffer,
			1u,
			vk::ShaderStageFlagBits::eVertex
		};
		descriptorSetLayout = device->createDescriptorSetLayout({
			{},
			1u,
			&binding
		});
	}

	void Renderer::updateUniformBuffer(uint32_t index) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		ubo.model = glm::rotate(
			glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = glm::perspective(
			glm::radians(45.0f),
			static_cast<float>(extent.width) / static_cast<float>(extent.height),
			0.1f, 10.0f);
		ubo.projection[1][1] *= -1;

		auto mappedData = allocator.mapMemory(uniformBuffers[index]);
		memcpy(mappedData, &ubo, sizeof(ubo));
		allocator.unmapMemory(uniformBuffers[index]);
	}

	void Renderer::createDescriptorPool() {
		descriptorPool = device->createDescriptorPool(vk::DescriptorType::eUniformBuffer,
			static_cast<uint32_t>(images.size()));
	}

	void Renderer::createDescriptorSets() {
		descriptorSets = device->allocateDescriptorSets(descriptorPool, descriptorSetLayout, static_cast<uint32_t>(images.size()));

		for (size_t i = 0; i < descriptorSets.size(); ++i) {
			vk::DescriptorBufferInfo bufferInfo{
				uniformBuffers[i],
				0u,
				sizeof(UniformBufferObject)
			};
			vk::WriteDescriptorSet descriptorWrite{
				descriptorSets[i],
				0u,
				0u,
				1u,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				&bufferInfo,
				nullptr
			};

			device->updateDescriptorSet(&descriptorWrite);
		}
	}
}
