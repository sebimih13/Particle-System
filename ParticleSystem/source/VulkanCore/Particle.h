#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <array>

namespace VulkanCore {

	struct Particle
	{
	public:
		glm::vec2 position;
		glm::vec2 velocity;
	};

} // namespace VulkanCore
