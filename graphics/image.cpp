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

	vk::ImageView Image::getView() {
		return view;
	}

	vk::ImageView* Image::setupAttachments(vk::ImageView depthImageView) {
		attachments = new vk::ImageView[2]{
			view,
			depthImageView
		};

		return attachments;
	}

	Image::~Image() {
		delete[] attachments;
	}

	Image::Image(vma::Allocator& allocator, Device* device, uint32_t width, uint32_t height, vk::Format format,
				 vk::ImageTiling tiling, vk::ImageUsageFlags const& imageUsage, vk::ImageAspectFlags const& aspectMask,
				 vma::MemoryUsage memoryUsage) {
		auto imageAllocation = allocator.createImage({
			{},
			vk::ImageType::e2D,
			format,
			{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u},
			1u,
			1u,
			vk::SampleCountFlagBits::e1,
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
			{aspectMask, 0u, 1u, 0u, 1u}
		});
	}

	Image::operator vk::Image() {
		return image;
	}
}
