#include "Renderer.h"

#include <stdexcept>

namespace VulkanCore {

	Renderer::Renderer(Window& window, GPUDevice& device)
		: window(window)
		, device(device)
		, currentImageIndex(0)
	{
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	Renderer::~Renderer()
	{
		// TODO: check
		// vkFreeCommandBuffers
		// commandBuffers.clear()
	}

	VkCommandBuffer Renderer::BeginFrame()
	{
		VkResult resultAcquireNextImage = swapChain->AcquireNextImage(&currentImageIndex);
		if (resultAcquireNextImage == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return nullptr;
		}
		else if (resultAcquireNextImage != VK_SUCCESS && resultAcquireNextImage != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		
		// TODO: ??? -> poate fi stearsa - ImGui Integration
		vkResetCommandBuffer(commandBuffers[swapChain->GetCurrentFrameIndex()], 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		// TODO: pentru ImGui
		beginInfo.pInheritanceInfo = nullptr;	// optional

		if (vkBeginCommandBuffer(commandBuffers[swapChain->GetCurrentFrameIndex()], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return commandBuffers[swapChain->GetCurrentFrameIndex()];
	}

	void Renderer::EndFrame()
	{
		if (vkEndCommandBuffer(commandBuffers[swapChain->GetCurrentFrameIndex()]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}

		VkResult resultSubmitCommandBuffer = swapChain->SubmitCommandBuffer(&commandBuffers[swapChain->GetCurrentFrameIndex()], &currentImageIndex);
		if (resultSubmitCommandBuffer == VK_ERROR_OUT_OF_DATE_KHR || resultSubmitCommandBuffer == VK_SUBOPTIMAL_KHR || window.GetWasWindowResized())
		{
			window.ResetWindowResizedFlag();
			RecreateSwapChain();
		}
		else if (resultSubmitCommandBuffer != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}

		swapChain->AdvanceFrameIndex();
	}

	void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		// TODO: check
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->GetRenderPass();
		renderPassInfo.framebuffer = swapChain->GetSwapChainFramebuffer(currentImageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

		VkClearValue clearColor = { };
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChain->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::CreateCommandBuffers()
	{
		commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = device.GetCommandPool();
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.GetVKDevice(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}

	void Renderer::RecreateSwapChain()
	{
		// Handling minimization
		int width = window.GetWidth();
		int height = window.GetHeight();
		while (width == 0 || height == 0)
		{
			width = window.GetWidth();
			height = window.GetHeight();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.GetVKDevice());

		swapChain.reset(nullptr);
		swapChain = std::make_unique<SwapChain>(device, window);
	}

} // namespace VulkanCore
