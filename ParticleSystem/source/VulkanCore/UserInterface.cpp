#include "UserInterface.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <stdexcept>
#include <array>
#include <iostream>

#include "SwapChain.h"

namespace VulkanCore {

	const uint32_t UserInterface::MAX_PARTICLE_MULTIPLIER = 131072; // TODO: change max value

	void UserInterface::HelpMarker(const std::string desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc.data());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

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
		, bShowMainMenuBar(true)
		, bShowSettingsWindow(false)
		, bShowGPUWindow(false)
		, bShowCPUWindow(false)
		, bShowRAMWindow(false)
		, bShowProgressBar(false)
		, particleCount(131072 * 64)
		, baseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
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

	void UserInterface::Update()
	{
		// Toggle Main Menu Bar - Shortcut
		static bool latestStateKeyALT = false;
		if (!latestStateKeyALT && ImGui::GetIO().KeyAlt)
		{
			bShowMainMenuBar = !bShowMainMenuBar;
		}
		latestStateKeyALT = ImGui::GetIO().KeyAlt;

		// Toggle Settings Window - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E))
		{
			bShowSettingsWindow = !bShowSettingsWindow;
		}

		// Toggle GPU Metrics Window - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
		{
			bShowGPUWindow = !bShowGPUWindow;
		}

		// Toggle CPU Metrics Window - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F))
		{
			bShowCPUWindow = !bShowCPUWindow;
		}

		// Toggle RAM Metrics Window - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R))
		{
			bShowRAMWindow = !bShowRAMWindow;
		}
	}

	void UserInterface::Draw(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// TODO: DELETE - demo window
		ImGui::ShowDemoWindow();

		DrawImGui();
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

	bool UserInterface::GetIsUIFocused() const
	{
		return ImGui::GetIO().WantCaptureMouse;
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

	void UserInterface::DrawImGui()
	{
		// Main Menu Bar
		if (bShowMainMenuBar) { ShowMainMenuBar(); }

		// UIs accessible from the "Settings" menu
		if (bShowSettingsWindow) { ShowSettingsWindow(); }

		// UIs accessible from the "Tools" menu
		if (bShowGPUWindow) { ShowGPUWindow(); }
		if (bShowCPUWindow) { ShowCPUWindow(); }
		if (bShowRAMWindow) { ShowRAMWindow(); }
	}

	void UserInterface::ShowMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::MenuItem("Open", "Ctrl+E", &bShowSettingsWindow);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("GPU Metrics", "Ctrl+C", &bShowGPUWindow);
				ImGui::MenuItem("CPU Metrics", "Ctrl+F", &bShowCPUWindow);
				ImGui::MenuItem("RAM Metrics", "Ctrl+R", &bShowRAMWindow);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void UserInterface::ShowSettingsWindow()
	{
		// We specify a default position/size in case there's no data in the .ini file
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(630, 200), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Settings", &bShowSettingsWindow))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// Particle count
		int particleMultiplier = particleCount / 64u;
		if (ImGui::SliderInt("##slider", &particleMultiplier, 1, MAX_PARTICLE_MULTIPLIER))
		{
			particleMultiplier = glm::clamp(particleMultiplier, 1, static_cast<int>(MAX_PARTICLE_MULTIPLIER));
		}

		ImGui::SameLine();
		float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

		ImGui::GetIO().KeyRepeatDelay = 0.75f;
		ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
		{
			// "-" Button
			if (ImGui::Button("-"))
			{
				if (particleMultiplier > 1)
				{
					particleMultiplier--;
				}
			}

			ImGui::SameLine(0.0f, spacing);

			// "+" Button
			if (ImGui::Button("+"))
			{
				if (particleMultiplier < MAX_PARTICLE_MULTIPLIER)
				{
					particleMultiplier++;
				}
			}
		}
		ImGui::PopItemFlag();
		ImGui::GetIO().KeyRepeatDelay = 0.275f;

		ImGui::SameLine();
		ImGui::Text("Particle Multiplier");
		ImGui::SameLine(); HelpMarker(
			"CTRL+Left-click on slider to input value.\n"
		);

		// Total number of particles
		particleCount = particleMultiplier * 64u;
		ImGui::Text("Particle Count: %d", particleCount);

		// Base color
		ImGui::ColorEdit4("Base color", &baseColor[0], ImGuiColorEditFlags_Float);
		ImGui::SameLine(); HelpMarker(
			"Click on the color square to open a color picker.\n"
			"Click and hold to use drag and drop.\n"
			"Right-click on the color square to show options.\n"
			"CTRL+click on individual component to input value.\n"
		);

		// Apply Button
		if (ImGui::Button("Apply"))
		{
			std::cout << "TODO: APPLY" << std::endl;
			bShowProgressBar = !bShowProgressBar; // TODO: change
		}
		ImGui::SameLine(); HelpMarker(
			"Left-click to apply the new changes and reload the simulation with the updated data.\n"
			"The simulation will refresh to reflect the new settings.\n"
		);

		// Load new data
		if (bShowProgressBar && ImGui::BeginTooltip())
		{
			// TODO: change
			ImGui::ProgressBar(sinf((float)ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(ImGui::GetFontSize() * 25, 0.0f));
			ImGui::EndTooltip();
		}

		// Main body of Settings Window ends here
		ImGui::End();
	}

	void UserInterface::ShowGPUWindow()
	{
		// We specify a default position/size in case there's no data in the .ini file
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + main_viewport->WorkSize.y - 210), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x - 20, 200), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("GPU Metrics", &bShowGPUWindow))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// TODO
		
		// Main body of GPU Metrics Window ends here
		ImGui::End();
	}

	void UserInterface::ShowCPUWindow()
	{
		// We specify a default position/size in case there's no data in the .ini file
		// TODO: default Window position

		if (!ImGui::Begin("CPU Metrics", &bShowCPUWindow))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// TODO

		// Main body of CPU Metrics Window ends here
		ImGui::End();
	}

	void UserInterface::ShowRAMWindow()
	{
		// We specify a default position/size in case there's no data in the .ini file
		// TODO: default Window position

		if (!ImGui::Begin("RAM Metrics", &bShowRAMWindow))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// TODO
		
		// Main body of RAM Metrics Window ends here
		ImGui::End();
	}

} // namespace VulkanCore
