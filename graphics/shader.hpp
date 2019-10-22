//
// Created by sabrina on 10/19/19.
//

#ifndef VULKAN_ENGINE_SHADER_HPP
#define VULKAN_ENGINE_SHADER_HPP

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include "device.hpp"

namespace Graphics {
	class Shader {
	public:
		Shader(std::string const& shaderName, Device* device, const vk::ShaderStageFlagBits& stage);
		vk::PipelineShaderStageCreateInfo getShaderStageCreateInfo();
	private:
		std::vector<char> code;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stage;
		static std::vector<char> loadShaderCode(std::string const& shaderName);
	};
}


#endif //VULKAN_ENGINE_SHADER_HPP
