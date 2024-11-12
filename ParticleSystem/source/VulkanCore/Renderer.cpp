#include "Renderer.h"

#include <stdexcept>

namespace VulkanCore {

	Renderer::Renderer(Window& window, GPUDevice& device)
		: window(window)
		, device(device)
	{
		RecreateSwapChain();
		pipeline = std::make_unique<Pipeline>(device, swapChain->GetRenderPass());	// TODO: move
		CreateCommandBuffers();
	}

	Renderer::~Renderer()
	{
		// TODO
	}

	// TODO: refactor
	void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;					// optional
		beginInfo.pInheritanceInfo = nullptr;	// optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->GetRenderPass();
		renderPassInfo.framebuffer = swapChain->GetSwapChainFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

		VkClearValue clearColor = { };
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			pipeline->Bind(commandBuffer);

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

			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	// TODO: refactor
	void Renderer::DrawFrame()
	{
		uint32_t imageIndex;
		VkResult resultAcquireNextImage = swapChain->AcquireNextImage(&imageIndex);
		if (resultAcquireNextImage == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (resultAcquireNextImage != VK_SUCCESS && resultAcquireNextImage != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		vkResetCommandBuffer(commandBuffers[swapChain->GetCurrentFrameIndex()], 0);
		RecordCommandBuffer(commandBuffers[swapChain->GetCurrentFrameIndex()], imageIndex);

		VkResult resultSubmitCommandBuffer = swapChain->SubmitCommandBuffer(&commandBuffers[swapChain->GetCurrentFrameIndex()], &imageIndex);
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
