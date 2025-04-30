#include "UserInterface.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <stdexcept>
#include <array>

#include "SwapChain.h"

// TODO: Create a seperate VkRenderPass, VkCommandBuffer and VkFramebuffer

namespace VulkanCore {

#ifdef DEBUG
	void UserInterface::CheckImGuiVulkanResult(VkResult err)
	{
		if (err == VK_SUCCESS)
		{
			return;
		}

		if (err < 0)
		{
			throw std::runtime_error("[ImGui] Error: VkResult = " + err);
		}
	}
#endif // DEBUG

	UserInterface::UserInterface(Window& window, GPUDevice& device, Renderer& renderer)
		: window(window)
		, device(device)
		, renderer(renderer)
	{
		CreateDescriptorPool();
		SetupImGui();
	}

	UserInterface::~UserInterface()
	{
		// ImGui cleanup
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UserInterface::Draw(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// TODO: our window
		ImGui::ShowDemoWindow();

		ImGui::Render();

		BeginRenderPass(commandBuffer);
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

			// Update and Render additional Platform Windows
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}
		EndRenderPass(commandBuffer);
	}

	void UserInterface::CreateDescriptorPool()
	{
		imGuiDescriptorPool = DescriptorPool::Builder(device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)									// required for ImGui backend
			.Build();
	}

	void UserInterface::SetupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(window.GetGLFWWindow(), true);
		ImGui_ImplVulkan_InitInfo initInfoImGui = {};
		initInfoImGui.Instance = device.GetInstance();
		initInfoImGui.PhysicalDevice = device.GetPhysicalDevice();
		initInfoImGui.Device = device.GetVKDevice();
		initInfoImGui.QueueFamily = device.GetPhysicalQueueFamilies().graphicsAndComputeFamily.value();
		initInfoImGui.Queue = device.GetGraphicsQueue();
		initInfoImGui.PipelineCache = VK_NULL_HANDLE;
		initInfoImGui.DescriptorPool = imGuiDescriptorPool->GetDescriptorPool();
		initInfoImGui.RenderPass = renderer.GetSwapChain()->GetImGuiRenderPass();
		initInfoImGui.Subpass = 0;
		initInfoImGui.MinImageCount = 2;
		initInfoImGui.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfoImGui.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfoImGui.Allocator = nullptr;
#ifdef DEBUG
		initInfoImGui.CheckVkResultFn = CheckImGuiVulkanResult;
#endif // DEBUG
		ImGui_ImplVulkan_Init(&initInfoImGui);
	}

	void UserInterface::BeginRenderPass(VkCommandBuffer commandBuffer)
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderer.GetSwapChain()->GetImGuiRenderPass();
		renderPassInfo.framebuffer = renderer.GetCurrentImGuiFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = renderer.GetSwapChain()->GetSwapChainExtent();
		renderPassInfo.clearValueCount = 0;
		renderPassInfo.pClearValues = nullptr; // no clear

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Set Dynamic States: Viewport + Scissors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = renderer.GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void UserInterface::EndRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace VulkanCore
