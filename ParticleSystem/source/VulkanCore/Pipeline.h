#pragma once

#include <string>
#include <vector>
#include <memory>

// TODO: sau Forward Declarations
#include "GPUDevice.h"
#include "SwapChain.h"

namespace VulkanCore {

	class Pipeline final
	{
	public:
		// Constructor
		Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout);

		// Destructor
		~Pipeline();

		// Not copyable
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator = (const Pipeline&) = delete;

		// Not moveable
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator = (Pipeline&&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		
		// Getters
		inline VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }

	private:
		GPUDevice& device;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		static const std::vector<VkDynamicState> dynamicStates;

		static std::vector<char> ReadFile(const std::string& filePath);

		void CreatePipelineLayout(const VkDescriptorSetLayout& descriptorSetLayout);
		void CreateGraphicsPipeline(const VkRenderPass& renderPass);
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
	};

} // namespace VulkanCore
