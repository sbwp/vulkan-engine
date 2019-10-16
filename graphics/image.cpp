//
// Created by sabrina on 10/15/19.
//

#include "image.hpp"

namespace Graphics {
	Image::Image(vk::Image image, vk::ImageView imageView, vk::ImageView depthImageView)
		: image(image), attachments{imageView, depthImageView} {}

	vk::ImageView const* Image::getAttachments() const {
		return attachments.data();
	}
}
