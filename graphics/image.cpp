//
// Created by sabrina on 10/15/19.
//

#include "image.hpp"
#include "device.hpp"

namespace Graphics {
	Image::Image(vk::Image image, vk::ImageView view, vma::Allocation allocation)
		: image(image), view(view), allocation(allocation) {}

	Image::Image(vk::Image image, vk::ImageView view)
		: image(image), view(view) {}

	Image::Image(vma::Allocator& allocator, Device* device, uint32_t width, uint32_t height, uint32_t mipLevels,
				 vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags const& imageUsage,
				 vk::ImageAspectFlags const& aspectMask, vk::SampleCountFlagBits const& sampleCount,
				 vma::MemoryUsage memoryUsage) {
		auto imageAllocation = allocator.createImage({
			{},
			vk::ImageType::e2D,
			format,
			{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u},
			mipLevels,
			1u,
			sampleCount,
			tiling,
			imageUsage
		}, {
			{},
			memoryUsage
		});
		image = imageAllocation.first;
		allocation = imageAllocation.second;
		view = device->createImageView({
			{},
			image,
			vk::ImageViewType::e2D,
			format,
			{},
			{aspectMask, 0u, mipLevels, 0u, 1u}
		});
	}

	Image::~Image() {
		delete[] attachments;
	}

	Image::operator vk::Image() {
		return image;
	}

	Image::operator vk::ImageView() {
		return view;
	}
}
