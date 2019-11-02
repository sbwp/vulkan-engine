//
// Created by sabrina on 10/31/19.
//

#ifndef VULKAN_ENGINE_VERTEX_HPP
#define VULKAN_ENGINE_VERTEX_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Graphics {
	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static vk::VertexInputBindingDescription getBindingDescription();

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
	};
}

#endif //VULKAN_ENGINE_VERTEX_HPP
