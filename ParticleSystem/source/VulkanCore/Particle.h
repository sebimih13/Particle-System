#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <array>

// TODO: move into ENGINE/APPLICATION namespace
namespace VulkanCore {

	struct Particle
	{
	public:
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;

        static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
	};

} // namespace VulkanCore
