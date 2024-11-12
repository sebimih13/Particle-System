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

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void DrawFrame();

	private:
		Window& window;
		GPUDevice& device;
		std::unique_ptr<SwapChain> swapChain;
		std::unique_ptr<Pipeline> pipeline;

		std::vector<VkCommandBuffer> commandBuffers;

		void CreateCommandBuffers();
		void RecreateSwapChain();
	};

} // namespace VulkanCore
