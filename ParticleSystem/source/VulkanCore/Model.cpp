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

	Model::Data::Data(const std::vector<Vertex>& vertices)
		: vertices(vertices)
	{

	}

	Model::Model(GPUDevice& device, const Data& builder)
		: device(device)
	{
		CreateVertexBuffer(builder.vertices);
		// TODO: CreateIndexBuffer
	}

	Model::~Model()
	{
		// TODO: refactor
		vkDestroyBuffer(device.GetVKDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), vertexBufferMemory, nullptr);
	}

	void Model::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);
	}

	void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());

		// TODO: refactor -> move to GPUDevice

		// Create buffer
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device.GetVKDevice(), &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer!");
		}

		// Memory allocation
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device.GetVKDevice(), vertexBuffer, &memoryRequirements);
		
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = device.FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device.GetVKDevice(), &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate vertex buffer memory!");
		}

		// Filling the vertex buffer
		vkBindBufferMemory(device.GetVKDevice(), vertexBuffer, vertexBufferMemory, 0);

		void* data;
		vkMapMemory(device.GetVKDevice(), vertexBufferMemory, 0, bufferInfo.size, 0, &data);
			std::memcpy(data, vertices.data(), static_cast<size_t>(bufferInfo.size));
		vkUnmapMemory(device.GetVKDevice(), vertexBufferMemory);
	}

} // namespace VulkanCore
