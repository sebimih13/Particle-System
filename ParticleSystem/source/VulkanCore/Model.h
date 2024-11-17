#pragma once

// TODO: remove
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// TODO: Forward Declaration
#include "GPUDevice.h"

#include <vector>
#include <array>

namespace VulkanCore {

	class Model
	{
	public:
		struct Vertex
		{
			const glm::vec2 position;		// TODO: change
			const glm::vec3 color;			// TODO: change

			Vertex(const glm::vec2& position, const glm::vec3& color);

			static VkVertexInputBindingDescription GetBindingDescription();
			static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescription();
		};

		struct Data
		{
			const std::vector<Vertex> vertices;
			// TODO: const std::vector<uint32_t> indices;

			Data(const std::vector<Vertex>& vertices);
		};

		// Constructor
		Model(GPUDevice& device, const Data& builder);
		
		// Destructor
		~Model();

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		GPUDevice& device;

		// TODO: refactor
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		void CreateVertexBuffer(const std::vector<Vertex>& vertices);

		// TODO: refactors
		// TODO: create a new class Buffer
		// TODO: define in GPUDevice
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	};

} // namespace VulkanCore
