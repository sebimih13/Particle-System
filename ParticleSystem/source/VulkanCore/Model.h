#pragma once

#include <GLFW/glfw3.h>
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
			const glm::vec3 position;
			const glm::vec3 color;			// TODO: change to glm::vec4
			const glm::vec2 texCoord;

			Vertex(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texCoord);

			static VkVertexInputBindingDescription GetBindingDescription();
			static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription();
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
