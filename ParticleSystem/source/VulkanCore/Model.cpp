#include "Model.h"

#include <glm/gtx/hash.hpp>

#include <tiny_obj_loader.h>

#include <stdexcept>
#include <cstring>

namespace std {

	template<>
	struct hash<VulkanCore::Model::Vertex>
	{
		size_t operator() (const VulkanCore::Model::Vertex& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
				  ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};

} // namespace std

namespace VulkanCore {

	Model::Vertex::Vertex()
		: position()
		, color()
		, texCoord()
	{

	}

	Model::Vertex::Vertex(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texCoord)
		: position(position)
		, color(color)
		, texCoord(texCoord)
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

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	Model::Data::Data(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: vertices(vertices)
		, indices(indices)
	{

	}

	Model::Data::Data(const std::string& objFilePath)
	{
		// check objFilePath
		// TODO

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFilePath.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex;

				if (index.vertex_index >= 0)
				{
					vertex.position = glm::vec3(
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					);

					vertex.color = glm::vec3(
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					);
				}

				// TODO: add normal
				//if (index.normal_index >= 0)
				//{
				//	vertex.normal = glm::vec3(
				//		attrib.normals[3 * index.normal_index + 0],
				//		attrib.normals[3 * index.normal_index + 1],
				//		attrib.normals[3 * index.normal_index + 2]
				//	);
				//}

				if (index.texcoord_index >= 0)
				{
					vertex.texCoord = glm::vec2(
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					);
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
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

		device.CopyBuffer(stagingBuffer, vertexBuffer, bufferSize, device.GetGraphicsQueue());

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

		device.CopyBuffer(stagingBuffer, indexBuffer, bufferSize, device.GetGraphicsQueue());

		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

} // namespace VulkanCore
