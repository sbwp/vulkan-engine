//
// Created by sabrina on 11/2/19.
//

#ifndef VULKAN_ENGINE_BUFFER_HPP
#define VULKAN_ENGINE_BUFFER_HPP

#include <vma.hpp>

namespace Graphics {
	class Buffer {
	public:
		Buffer() = default;
		Buffer(vma::Allocator const& allocator, vk::DeviceSize size, const vk::BufferUsageFlags& bufferUsage,
			   const vk::MemoryPropertyFlags& memoryProperties, vma::MemoryUsage memoryUsage);

		operator vk::Buffer(); // NOLINT
		operator vma::Allocation(); // NOLINT
	private:
		vk::Buffer buffer;
		vma::Allocation allocation;
	};
}

#endif //VULKAN_ENGINE_BUFFER_HPP
