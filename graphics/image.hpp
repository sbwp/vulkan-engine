//
// Created by sabrina on 10/15/19.
//

#ifndef VULKAN_ENGINE_IMAGE_HPP
#define VULKAN_ENGINE_IMAGE_HPP

#include <vulkan/vulkan.hpp>

namespace Graphics {
	class Image {
	public:
		Image(vk::Image image, vk::ImageView imageView, vk::ImageView depthImageView);

		[[nodiscard]] vk::ImageView const* getAttachments() const;
	private:
		vk::Image image;
		std::array<vk::ImageView, 2> attachments;
	};
}
#endif //VULKAN_ENGINE_IMAGE_HPP
