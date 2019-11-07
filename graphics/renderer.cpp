//
// Created by sabrina on 10/1/19.
//

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <algorithm>
#include <chrono>

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"
#include "../util/algorithm.hpp"
#include "shader.hpp"
#include "uniform-buffer-object.hpp"

namespace Graphics {
	Renderer::Renderer(Core::Game& game): game(game), window(1820, 954, "Vulkan Engine"),
										  presentMode(vk::PresentModeKHR::eFifo), depthFormat(vk::Format::eUndefined),
										  ubo{} {
		glfw::appendRequiredExtensions(instanceExtensions);
		setupValidationLayers(instanceExtensions, validationLayers);
		createInstance();
		surface = window.createSurface(instance);
		choosePhysicalDevice();
		device->createLogicalDevice(deviceExtensions, validationLayers);
		allocator = device->createAllocator();
		createCommandPool();
		createTextureImage();
		createTextureSampler();
		createSynchronization();

		loadModel();
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
		createDepthImage();
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
				std::array<vk::ClearValue, 2> clearValues = {
					Util::makeClearColor(0.0f, 0.0f, 0.0f),
					Util::makeClearDepthStencil(1.0f, 0u)
				};
				vk::RenderPassBeginInfo renderPassBeginInfo{
					renderPass,
					framebuffers[i],
					{{0, 0}, extent},
					static_cast<uint32_t>(clearValues.size()),
					clearValues.data()
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
		uint32_t queueIndices[] = {device->graphicsIndex(), device->presentIndex()};
		bool queuesSame = queueIndices[0] == queueIndices[1];

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
			queueIndices,
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

		// Aspect Mask always has depth bit, only has stencil bit if supported by format
		vk::ImageAspectFlags aspectMask = Util::doesFormatSupportStencil(depthFormat)
										  ? vk::ImageAspectFlagBits::eDepth
										  : vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

		depthImage = Image(allocator, device, extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment, aspectMask,
			vma::MemoryUsage::eGpuOnly);

		transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal);
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
		std::vector<vk::AttachmentDescription> attachments{
			{   // Color Attachment
				vk::AttachmentDescriptionFlags{},
				surfaceFormat.format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			},
			{   // Depth Attachment
				vk::AttachmentDescriptionFlags{},
				depthFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			}
		};

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
		}, images, depthImage);
	}

	void Renderer::createTextureImage() {
		int width, height, channels;
		stbi_uc* pixels = stbi_load("../assets/textures/chalet.jpg", &width, &height, &channels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = width * height * 4;

		if (!pixels) {
			throw Logger::error("Failed to load image");
		}

		auto stagingBuffer = Buffer(allocator, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			vma::MemoryUsage::eCpuToGpu);

		copyMemory(stagingBuffer, pixels, static_cast<size_t>(imageSize));

		stbi_image_free(pixels);

		textureImage = Image(allocator, device, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
			vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			vma::MemoryUsage::eGpuOnly);

		transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal);
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
			true,
			true,
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

		copyMemory(stagingBuffer, vertices.data(), sizeof(vertices[0]) * vertices.size());

		copyBuffer(stagingBuffer, vertexBuffer, size);
	}

	void Renderer::createIndexBuffer() {
		vk::DeviceSize size = sizeof(indices[0]) * indices.size();

		Buffer stagingBuffer(allocator, size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			vma::MemoryUsage::eCpuToGpu);

		copyMemory(stagingBuffer, indices.data(), sizeof(indices[0]) * indices.size());

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
		runCommand([&](vk::CommandBuffer const& commandBuffer) {
			vk::BufferCopy copyRegion{0, 0, size};
			commandBuffer.copyBuffer(src, dst, 1u, &copyRegion);
		});
	}

	void Renderer::createDescriptorSetLayout() {
		std::vector<vk::DescriptorSetLayoutBinding> bindings{
			{
				0u,
				vk::DescriptorType::eUniformBuffer,
				1u,
				vk::ShaderStageFlagBits::eVertex
			},
			{
				1u,
				vk::DescriptorType::eCombinedImageSampler,
				1u,
				vk::ShaderStageFlagBits::eFragment
			}
		};
		descriptorSetLayout = device->createDescriptorSetLayout({
			{},
			static_cast<uint32_t>(bindings.size()),
			bindings.data()
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

		copyMemory(uniformBuffers[index], &ubo, sizeof(ubo));
	}

	void Renderer::createDescriptorPool() {
		descriptorPool = device->createDescriptorPool(
			static_cast<uint32_t>(images.size()));
	}

	void Renderer::createDescriptorSets() {
		descriptorSets = device
			->allocateDescriptorSets(descriptorPool, descriptorSetLayout, static_cast<uint32_t>(images.size()));

		for (size_t i = 0; i < descriptorSets.size(); ++i) {
			vk::DescriptorBufferInfo bufferInfo{
				uniformBuffers[i],
				0u,
				sizeof(UniformBufferObject)
			};
			vk::DescriptorImageInfo imageInfo{
				textureSampler,
				textureImage,
				vk::ImageLayout::eShaderReadOnlyOptimal
			};
			std::vector<vk::WriteDescriptorSet> writeDescriptorSets{
				{
					descriptorSets[i],
					0u,
					0u,
					1u,
					vk::DescriptorType::eUniformBuffer,
					nullptr,
					&bufferInfo,
					nullptr
				},
				{
					descriptorSets[i],
					1u,
					0u,
					1u,
					vk::DescriptorType::eCombinedImageSampler,
					&imageInfo,
					nullptr,
					nullptr
				}
			};

			device->updateDescriptorSets(writeDescriptorSets);
		}
	}

	void Renderer::copyMemory(vma::Allocation const& allocation, void* data, size_t size) {
		auto mappedMemory = allocator.mapMemory(allocation);
		memcpy(mappedMemory, data, size);
		allocator.unmapMemory(allocation);
	}

	void Renderer::runCommand(std::function<void(vk::CommandBuffer)> const& callback) {
		auto commandBuffer = device->allocateCommandBuffers({
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			1u
		})[0];
		commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
		callback(commandBuffer);
		commandBuffer.end();

		vk::SubmitInfo submitInfo{
			0u, nullptr, nullptr, 1u, &commandBuffer
		};
		device->graphicsQueue.submit(1u, &submitInfo, vk::Fence{});
	}

	void Renderer::transitionImageLayout(Image& image, vk::Format const& format, vk::ImageLayout const& from,
										 vk::ImageLayout const& to) {
		runCommand([&](vk::CommandBuffer const& commandBuffer) {
			vk::ImageMemoryBarrier barrier{
				accessMaskForLayout(from), accessMaskForLayout(to),
				from, to,
				0u, 0u,
				image,
				{aspectMaskForLayoutAndFormat(to, format), 0u, 1u, 0u, 1u}
			};
			commandBuffer.pipelineBarrier(pipelineStageForLayout(from), pipelineStageForLayout(to), {},
				0u, nullptr,
				0u, nullptr,
				1u, &barrier);
		});
	}

	void Renderer::copyBufferToImage(Buffer& buffer, Image& image, uint32_t width, uint32_t height) {
		runCommand([&](vk::CommandBuffer const& commandBuffer) {
			vk::BufferImageCopy copy{
				0u, 0u, 0u,
				{vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u},
				{0u, 0u, 0u},
				{width, height, 1u}
			};
			commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1u, &copy);
		});
	}

	vk::AccessFlags Renderer::accessMaskForLayout(vk::ImageLayout const& layout) {
		switch (layout) {
			case vk::ImageLayout::eTransferDstOptimal:
				return vk::AccessFlagBits::eTransferWrite;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				return vk::AccessFlagBits::eShaderRead;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				return vk::AccessFlagBits::eDepthStencilAttachmentRead |
					vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			default:
				return vk::AccessFlags{};
		}
	}

	vk::PipelineStageFlags Renderer::pipelineStageForLayout(vk::ImageLayout const& layout) {
		switch (layout) {
			case vk::ImageLayout::eUndefined:
				return vk::PipelineStageFlagBits::eTopOfPipe;
			case vk::ImageLayout::eTransferDstOptimal:
				return vk::PipelineStageFlagBits::eTransfer;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				return vk::PipelineStageFlagBits::eFragmentShader;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				return vk::PipelineStageFlagBits::eEarlyFragmentTests;
			default:
				return vk::PipelineStageFlags{};
		}
	}

	void Renderer::createTextureSampler() {
		textureSampler = device->createSampler({
			{},
			vk::Filter::eLinear,
			vk::Filter::eLinear,
			vk::SamplerMipmapMode::eNearest,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			0.0f, true, 16.0f, false, vk::CompareOp::eAlways,
			0.0f, 0.0f, vk::BorderColor::eIntOpaqueBlack, false
		});
	}

	vk::ImageAspectFlags Renderer::aspectMaskForLayoutAndFormat(vk::ImageLayout const& layout,
																vk::Format const& format) {
		if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			if (Util::doesFormatSupportStencil(format)) {
				return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			} else {
				return vk::ImageAspectFlagBits::eDepth;
			}
		} else {
			return vk::ImageAspectFlagBits::eColor;
		}
	}

	// TODO: Pull out into separate model/mesh class
	void Renderer::loadModel() {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		static std::string const path = "../assets/models/chalet.obj";
		Logger::assertTrue(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()),
			"Failed to load object: " + warn + err);
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				vertices.emplace_back(
					glm::vec3{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
					    attrib.vertices[3 * index.vertex_index + 2]
					},
					glm::vec3{ 1.0f, 1.0f, 1.0f },
					glm::vec2{
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					}
				);
				indices.push_back(indices.size()); // TODO: Move vertices to unordered_map to remove duplicates
			}
		}
	}
}
