#pragma once

// TODO: sau Forward Declarations
#include "GPUDevice.h"
#include "SwapChain.h"

#include <string>
#include <vector>

namespace VulkanCore {

	class Pipeline final
	{
	public:
		// Constructor
		Pipeline(GPUDevice& device, SwapChain& swapChain);	// TODO: refactor

		// Destructor
		~Pipeline();

		// Not copyable
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator = (const Pipeline&) = delete;

		// Not moveable
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator = (Pipeline&&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

	private:
		GPUDevice& device;
		SwapChain& swapChain;

		VkPipeline graphicsPipeline;

		VkPipelineLayout pipelineLayout; // TODO: nullptr

		static const std::vector<VkDynamicState> dynamicStates;

		static std::vector<char> ReadFile(const std::string& filePath);

		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
	};

} // namespace VulkanCore
