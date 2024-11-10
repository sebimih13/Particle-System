#pragma once

// TODO: sau Forward Declarations
#include "GPUDevice.h"

// TODO: remove
#include "SwapChain.h"

#include <string>
#include <vector>

namespace VulkanCore {

	class Pipeline
	{
	public:
		// Constructor
		Pipeline(GPUDevice& device, SwapChain& swapChain);

		// Destructor
		~Pipeline();

		// Not copyable
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator = (const Pipeline&) = delete;

		// Not moveable
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator = (Pipeline&&) = delete;

	private:
		GPUDevice& device;

		VkPipelineLayout pipelineLayout;

		static const std::vector<VkDynamicState> dynamicStates;

		static std::vector<char> ReadFile(const std::string& filePath);

		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
	};

} // namespace VulkanCore
