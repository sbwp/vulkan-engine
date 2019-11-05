//
// Created by sabrina on 10/19/19.
//

#include "shader.hpp"
#include "../util/algorithm.hpp"

namespace Graphics {
	Shader::Shader(std::string const& shaderName, Device* device, const vk::ShaderStageFlagBits& stage):
		code{loadShaderCode(shaderName)}, shaderModule{device->createShaderModule(code)}, stage(stage) {}

	std::vector<char> Shader::loadShaderCode(std::string const& shaderName) {
		return Util::readFile("shaders/" + shaderName + ".spv");
	}

	vk::PipelineShaderStageCreateInfo Shader::getShaderStageCreateInfo() {
		return {
			{},
			stage,
			shaderModule,
			"main"
		};
	}

}