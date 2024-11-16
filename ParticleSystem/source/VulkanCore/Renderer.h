#pragma once

// TODO: forward declarations?
#include "Window.h"
#include "GPUDevice.h"
#include "SwapChain.h"
#include "Pipeline.h"

#include <memory>

namespace VulkanCore {

	class Renderer
	{
	public:
		// Constructor
		Renderer(Window& window, GPUDevice& device); // TODO: refactor

		// Destructor
		~Renderer();

		// Not copyable
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		// Not moveable
		Renderer(Renderer&&) = delete;
		Renderer& operator = (Renderer&&) = delete;

		// TODO: test
		VkCommandBuffer BeginFrame();
		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		Window& window;
		GPUDevice& device;
		std::unique_ptr<SwapChain> swapChain;
		std::unique_ptr<Pipeline> pipeline;		// TODO: move

		std::vector<VkCommandBuffer> commandBuffers;
		uint32_t currentImageIndex;

		void CreateCommandBuffers();
		void RecreateSwapChain();
	};

} // namespace VulkanCore
