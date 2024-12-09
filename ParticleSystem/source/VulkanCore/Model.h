#pragma once

// TODO: remove
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <array>

// TODO: Forward Declaration
#include "GPUDevice.h"

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
			const std::vector<uint32_t> indices;

			Data(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
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

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
	};

} // namespace VulkanCore
