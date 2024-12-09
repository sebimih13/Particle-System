#include "Model.h"

#include <stdexcept>
#include <cstring>

namespace VulkanCore {

	Model::Vertex::Vertex(const glm::vec2& position, const glm::vec3& color)
		: position(position)
		, color(color)
	{

	}

	VkVertexInputBindingDescription Model::Vertex::GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 2> Model::Vertex::GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

	Model::Data::Data(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: vertices(vertices)
		, indices(indices)
	{

	}

	Model::Model(GPUDevice& device, const Data& builder)
		: device(device)
		, vertexCount(0)
		, indexCount(0)
	{
		CreateVertexBuffer(builder.vertices);
		CreateIndexBuffer(builder.indices);
	}

	Model::~Model()
	{
		// TODO: refactor
		vkDestroyBuffer(device.GetVKDevice(), indexBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(device.GetVKDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), vertexBufferMemory, nullptr);
	}

	void Model::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}

	void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		
		// Create staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		// Filling staging buffer
		void* data;
		vkMapMemory(device.GetVKDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.GetVKDevice(), stagingBufferMemory);

		// Create vertex buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		device.CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

	void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		// Create staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		// Filling staging buffer
		void* data;
		vkMapMemory(device.GetVKDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.GetVKDevice(), stagingBufferMemory);

		// Create index buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory
		);

		device.CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

} // namespace VulkanCore
