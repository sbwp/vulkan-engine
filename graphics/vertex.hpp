//
// Created by sabrina on 10/31/19.
//

#ifndef VULKAN_ENGINE_VERTEX_HPP
#define VULKAN_ENGINE_VERTEX_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Graphics {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texCoord);

		static vk::VertexInputBindingDescription getBindingDescription();

		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();

		bool operator==(const Vertex& other) const;
	};
}

namespace std {
	template<> struct hash<Graphics::Vertex> {
		size_t operator()(Graphics::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^ //NOLINT
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ //NOLINT
				(hash<glm::vec2>()(vertex.texCoord) << 1); //NOLINT
		}
	};
}

#endif //VULKAN_ENGINE_VERTEX_HPP
