#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
		uint32_t currentImageIndex;

		void CreateCommandBuffers();
		void RecreateSwapChain();
	};

} // namespace VulkanCore
