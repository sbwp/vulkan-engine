//
// Created by sabrina on 11/2/19.
//

#ifndef VULKAN_ENGINE_UNIFORM_BUFFER_OBJECT_HPP
#define VULKAN_ENGINE_UNIFORM_BUFFER_OBJECT_HPP

#include <glm/glm.hpp>

namespace Graphics {
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};
}

#endif //VULKAN_ENGINE_UNIFORM_BUFFER_OBJECT_HPP
