//
// Created by sabrina on 11/2/19.
//

#include "buffer.hpp"

namespace Graphics {
	Buffer::Buffer(vma::Allocator const& allocator, vk::DeviceSize size, const vk::BufferUsageFlags& bufferUsage,
				   const vk::MemoryPropertyFlags& memoryProperties, vma::MemoryUsage memoryUsage) {
		auto allocationPair = allocator.createBuffer({
			{},
			size,
			bufferUsage,
			vk::SharingMode::eExclusive
		}, {
			{},
			memoryUsage,
			memoryProperties
		});

		buffer = allocationPair.first;
		allocation = allocationPair.second;
	}

	Buffer::operator vk::Buffer() {
		return buffer;
	}

	Buffer::operator vma::Allocation() {
		return allocation;
	}
}
