//
// Created by sabrina on 11/2/19.
//

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.hpp>
#include "texture.hpp"
#include "../logger/logger.hpp"

Graphics::Texture::Texture(char const* filename) {
	int width, height, channels;
	stbi_uc* pixels = stbi_load("../assets/images/sabrina.jpg", &width, &height, &channels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = width * height * 4;

	if (!pixels) {
		throw Logger::error("Failed to load image");
	}

	stbi_image_free(pixels);
}
