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

	vk::ClearValue makeClearDepthStencil(float depth, uint32_t stencil) {
		return vk::ClearValue{vk::ClearDepthStencilValue{depth, stencil}};
	}

	bool doesFormatSupportStencil(vk::Format const& format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	vk::SampleCountFlagBits maxSampleCount(vk::SampleCountFlags const& supportedSampleCounts) {
		return supportedSampleCounts & vk::SampleCountFlagBits::e64 ? vk::SampleCountFlagBits::e64
			   : supportedSampleCounts & vk::SampleCountFlagBits::e32 ? vk::SampleCountFlagBits::e32
				 : supportedSampleCounts & vk::SampleCountFlagBits::e16 ? vk::SampleCountFlagBits::e16
				   : supportedSampleCounts & vk::SampleCountFlagBits::e8 ? vk::SampleCountFlagBits::e8
					 : supportedSampleCounts & vk::SampleCountFlagBits::e4 ? vk::SampleCountFlagBits::e4
					   : supportedSampleCounts & vk::SampleCountFlagBits::e2 ? vk::SampleCountFlagBits::e2
						 : vk::SampleCountFlagBits::e1;
	}

}