//
// Created by sabrina on 10/18/19.
//

#include "algorithm.hpp"
#include "../logger/logger.hpp"

namespace Util {
	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if(!file.is_open()) {
			throw Logger::error("failed to load file " + filename);
		}

		auto fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	vk::ClearValue makeClearColor(float r, float g, float b, float a) {
		return vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{r, g, b, a}}};
	}

	vk::ClearValue makeClearColor(float r, float g, float b) {
		return makeClearColor(r, g, b, 1.0f);
	}
}