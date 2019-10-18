//
// Created by sabrina on 10/15/19.
//

#include "image.hpp"

namespace Graphics {
	Image::Image(vk::Image image, vk::ImageView view, vma::Allocation allocation)
		: image(image), view(view), allocation(allocation), attachments(nullptr) {}

	Image::Image(vk::Image image, vk::ImageView view)
		: image(image), view(view), attachments(nullptr) {}

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
}
