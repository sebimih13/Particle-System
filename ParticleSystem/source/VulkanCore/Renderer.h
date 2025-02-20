#pragma once

#include <glm/glm.hpp>

#include <memory>

// TODO: forward declarations?
#include "Window.h"
#include "GPUDevice.h"
#include "SwapChain.h"
#include "Pipeline.h"

namespace VulkanCore {

	class Renderer
	{
	public:
		// Constructor
		Renderer(Window& window, GPUDevice& device);

		// Destructor
		~Renderer();

		// Not copyable
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		// Not moveable
		Renderer(Renderer&&) = delete;
		Renderer& operator = (Renderer&&) = delete;

		VkCommandBuffer BeginFrame();
		void EndFrame();

		VkCommandBuffer BeginCompute();
		void EndCompute();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

		// Getters
		// inline const SwapChain* const GetSwapChain() const { return swapChain.get(); }		// TODO: nu cred ca e suficient de safe
		// TODO: inline const std::unique_ptr<SwapChain>& GetSwapChain() const { return swapChain; }
		inline const std::unique_ptr<SwapChain>& GetSwapChain() const { return swapChain; }

	private:
		Window& window;
		GPUDevice& device;
		std::unique_ptr<SwapChain> swapChain;

		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkCommandBuffer> computeCommandBuffers;
		uint32_t currentImageIndex;

		void CreateCommandBuffers();
		void CreateComputeCommandBuffers();
		void RecreateSwapChain();
	};

} // namespace VulkanCore
