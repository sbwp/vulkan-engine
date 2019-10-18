//
// Created by sabrina on 10/15/19.
//

#ifndef VULKAN_ENGINE_IMAGE_HPP
#define VULKAN_ENGINE_IMAGE_HPP

#include <optional>
#include <vulkan/vulkan.hpp>
#include <vma.hpp>

namespace Graphics {
	class Image {
	public:
		Image() = default;
		Image (vk::Image image, vk::ImageView view, vma::Allocation allocation);
		Image (vk::Image image, vk::ImageView view);
		~Image();

		vk::ImageView getView();
		vk::ImageView* setupAttachments(vk::ImageView depthImageView);
	private:
		vk::Image image;
		vk::ImageView view;
		std::optional<vma::Allocation> allocation;
		vk::ImageView* attachments;
	};
}
#endif //VULKAN_ENGINE_IMAGE_HPP
