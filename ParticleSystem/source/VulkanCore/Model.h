#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <string>

#include "GPUDevice.h"

namespace VulkanCore {

	class Model
	{
	public:
		struct Vertex
		{
			// Attributes
			glm::vec3 position;
			glm::vec3 color;
			glm::vec2 texCoord;

			// Constructors
			Vertex();
			Vertex(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texCoord);

			// Operator ==
			bool operator == (const Vertex& other) const { return position == other.position && color == other.color && texCoord == other.texCoord; }

			static VkVertexInputBindingDescription GetBindingDescription();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
		};

		struct Data
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			// Constructors
			Data(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
			Data(const std::string& objFilePath);
		};

		// Constructor
		Model(GPUDevice& device, const Data& builder);
		
		// Destructor
		~Model();

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		GPUDevice& device;

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
