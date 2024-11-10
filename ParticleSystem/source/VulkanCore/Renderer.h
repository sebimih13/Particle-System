#pragma once

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
		Renderer(Window& window, GPUDevice& device, SwapChain& swapChain, Pipeline& pipeline); // TODO: refactor

		// Destructor
		~Renderer();

		// Not copyable
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		// Not moveable
		Renderer(Renderer&&) = delete;
		Renderer& operator = (Renderer&&) = delete;

		void BeginFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		Window& window;
		GPUDevice& device;
		SwapChain& swapChain;
		Pipeline& pipeline;		// TODO: move

		VkCommandBuffer commandBuffer;

		void CreateCommandBuffer();
	};

} // namespace VulkanCore
