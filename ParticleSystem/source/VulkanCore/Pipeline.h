#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

// TODO: sau Forward Declarations
#include "GPUDevice.h"
#include "SwapChain.h"

namespace VulkanCore {

	class Pipeline final
	{
	public:
		// Constructor
		Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout, const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
		Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout, const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& computeShaderFilePath);

		// Destructor
		~Pipeline();

		// Not copyable
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator = (const Pipeline&) = delete;

		// Not moveable
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator = (Pipeline&&) = delete;

		void BindComputePipeline(VkCommandBuffer commandBuffer);
		void BindGraphicsPipeline(VkCommandBuffer commandBuffer);
		
		// Getters
		inline VkPipelineLayout GetGraphicsPipelineLayout() const { return pipelineLayout; }
		inline VkPipelineLayout GetComputePipelineLayout() const { return computePipelineLayout; }

	private:
		GPUDevice& device;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		bool hasComputePipeline;
		VkPipelineLayout computePipelineLayout;
		VkPipeline computePipeline;

		static const std::vector<VkDynamicState> dynamicStates;

		static std::vector<char> ReadFile(const std::string& filePath);

		void CreateGraphicsPipeline(const VkRenderPass& renderPass, const std::optional<VkDescriptorSetLayout>& descriptorSetLayout, const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
		void CreateComputePipeline(const VkDescriptorSetLayout& descriptorSetLayout, const std::string& computeShaderFilePath);
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
	};

} // namespace VulkanCore
