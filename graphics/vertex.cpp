//
// Created by sabrina on 10/31/19.
//

#include "vertex.hpp"

namespace Graphics {
	vk::VertexInputBindingDescription Vertex::getBindingDescription() {
		return vk::VertexInputBindingDescription{
			0,
			sizeof(Vertex),
			vk::VertexInputRate::eVertex
		};
	}

	std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
		return std::array<vk::VertexInputAttributeDescription, 3>{{
			{0u, 0u, vk::Format::eR32G32B32Sfloat, 0u},
			{1u, 0u, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)},
			{2u, 0u, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)}
		}};
	}

	Vertex::Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texCoord): position(position), color(color),
																			 texCoord(texCoord) {}

	bool Vertex::operator==(const Vertex& other) const {
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}
}
