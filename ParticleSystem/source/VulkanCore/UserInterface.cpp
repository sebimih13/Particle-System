#include "UserInterface.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui_internal.h>

#include <stdexcept>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <algorithm>
#include <numeric>

#include "SwapChain.h"
#include "FrameTimeHistory.h"
#include "FPSCounter.h"

namespace VulkanCore {

	ImChunkStream<UserInterface::ImGuiWindowUserData> UserInterface::UserDataWindows;

	const uint32_t UserInterface::MAX_PARTICLE_MULTIPLIER = 131072; // TODO: change max value

	std::unordered_map<std::string, bool> UserInterface::UserDataWindow = {
		{ "Settings", false },
		{ "GPU Metrics", false }
	};

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

	UserInterface::UserInterface(Window& window, InputManager& inputManager, GPUDevice& device, Renderer& renderer)
		: window(window)
		, inputManager(inputManager)
		, device(device)
		, renderer(renderer)
		, bShowMainMenuBar(true)
		, bShouldReset(false)
		, bCaptureInput(false)
		, bInBenchmark(false)
		, particleCount(131072 * 64)
		, staticColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f))
		, dynamicColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f))
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
			UserDataWindow["Settings"] = !UserDataWindow["Settings"];
		}

		// Toggle GPU Metrics Window - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
		{
			UserDataWindow["GPU Metrics"] = !UserDataWindow["GPU Metrics"];
		}

		// Toggle Capture Input Command - Shortcut
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R))
		{
			bCaptureInput = !bCaptureInput;
		}
	}

	void UserInterface::Draw(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

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

	void UserInterface::ToggleShouldReset()
	{
		bShouldReset = !bShouldReset;
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

		// Add .ini handle for UserData type
		{
			ImGuiSettingsHandler ini_handler;
			ini_handler.TypeName = "UserData";
			ini_handler.TypeHash = ImHashStr("UserData");
			ini_handler.ReadOpenFn = UserData_ReadOpen;
			ini_handler.ReadLineFn = UserData_ReadLine;
			ini_handler.WriteAllFn = UserData_WriteAll;
			ImGui::AddSettingsHandler(&ini_handler);
		}

		// ImGui Flags
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
		if (UserDataWindow["Settings"]) { ShowSettingsWindow(); }

		// UIs accessible from the "Tools" menu
		if (UserDataWindow["GPU Metrics"]) { ShowGPUWindow(); }
	}

	void UserInterface::ShowMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::MenuItem("Open", "Ctrl+E", &UserDataWindow["Settings"]);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("GPU Metrics", "Ctrl+C", &UserDataWindow["GPU Metrics"]);
				ImGui::MenuItem("Capture Input", "Ctrl+R", &bCaptureInput);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Benchmark"))
			{
				if (ImGui::MenuItem("Test 1", nullptr, nullptr)) { StartBenchmark(Benchmark::Test1); }
				if (ImGui::MenuItem("Test 2", nullptr, nullptr)) { StartBenchmark(Benchmark::Test2); }
				if (ImGui::MenuItem("Test 3", nullptr, nullptr)) { StartBenchmark(Benchmark::Test3); }
				if (ImGui::MenuItem("Test 4", nullptr, nullptr)) { StartBenchmark(Benchmark::Test4); }
				if (ImGui::MenuItem("Test 5", nullptr, nullptr)) { StartBenchmark(Benchmark::Test5); }
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

		if (!ImGui::Begin("Settings", &UserDataWindow["Settings"]))
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

		// Colors
		ImGui::ColorEdit4("Static color", &staticColor[0], ImGuiColorEditFlags_Float);
		ImGui::SameLine(); HelpMarker(
			"This color represents the particle at rest, with minimal or no velocity.\n"
			"Click on the color square to open a color picker.\n"
			"Click and hold to use drag and drop.\n"
			"Right-click on the color square to show options.\n"
			"CTRL+click on individual component to input value.\n"
		);

		ImGui::ColorEdit4("Dynamic color", &dynamicColor[0], ImGuiColorEditFlags_Float);
		ImGui::SameLine(); HelpMarker(
			"This color represents the particle in motion, with higher velocity and increased energy.\n"
			"Click on the color square to open a color picker.\n"
			"Click and hold to use drag and drop.\n"
			"Right-click on the color square to show options.\n"
			"CTRL+click on individual component to input value.\n"
		);

		// Apply Button
		if (ImGui::Button("Apply"))
		{
			ToggleShouldReset();
		}
		ImGui::SameLine(); HelpMarker(
			"Left-click to apply the new changes and reload the simulation with the updated data.\n"
			"The simulation will refresh to reflect the new settings.\n"
		);

		// Main body of Settings Window ends here
		ImGui::End();
	}

	void UserInterface::ShowGPUWindow()
	{
		// We specify a default position/size in case there's no data in the .ini file
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + main_viewport->WorkSize.y - 210), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x - 20, 200), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("GPU Metrics", &UserDataWindow["GPU Metrics"]))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		ImGui::Text("GPU used: %s", device.GetName().data());

		// FPS Graph
		static std::vector<float> fpsHistory;
		const uint32_t maxFpsHistory = 240; // TODO

		fpsHistory.push_back(FPSCounter::GetInstance().GetFPS());
		if (fpsHistory.size() > maxFpsHistory)
		{
			fpsHistory.erase(fpsHistory.begin());
		}

		ImGui::Text("FPS: %.2f", FPSCounter::GetInstance().GetFPS());
		ImGui::SameLine();
		ImGui::PlotLines("###FPS", fpsHistory.data(), static_cast<int>(fpsHistory.size()), 0, nullptr, 0.0f, static_cast<float>(maxFpsHistory), ImVec2(0, 80));

		// Frame Time Graph
		const float width = ImGui::GetWindowWidth();
		const size_t frameCount = FrameTimeHistory::GetInstance().GetCount();
		if (width > 0.0f && frameCount > 0)
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 basePos = ImGui::GetCursorScreenPos();

			constexpr float minHeight = 2.0f;
			constexpr float maxHeight = 64.0f;

			constexpr float minDT = 1.0f / 120.0f;	// TODO: modifica MAX_FPS
			constexpr float maxDT = 1.0f / 15.0f;
			
			const float log2MinDT = log2(minDT);
			const float log2MaxDT = log2(maxDT);
			
			float endX = width;
			drawList->AddRectFilled(basePos, ImVec2(basePos.x + width, basePos.y + maxHeight), 0xFF404040);
			
			for (size_t frameIndex = 0; frameIndex < frameCount && endX > 0.0f; ++frameIndex)
			{
				const FrameTimeHistory::Entry dt = FrameTimeHistory::GetInstance().GetEntry(frameIndex);
				const float frameWidth = dt.deltaTime / minDT;
				const float frameHeightFactor = (dt.log2DeltaTime - log2MinDT) / (log2MaxDT - log2MinDT);
				const float frameHeightFactorClamped = std::min(std::max(0.0f, frameHeightFactor), 1.0f);
				const float frameHeight = glm::mix(minHeight, maxHeight, frameHeightFactorClamped);
				const float begX = endX - frameWidth;
				const uint32_t color = glm::packUnorm4x8(DeltaTimeToColor(dt.deltaTime));

				drawList->AddRectFilled(
					ImVec2(basePos.x + std::max(0.0f, std::floor(begX)), basePos.y + maxHeight - frameHeight),
					ImVec2(basePos.x + std::ceil(endX), basePos.y + maxHeight),
					color
				);

				endX = begX;
			}

			ImGui::Text("Frame time: %.2f ms", SecondsToMiliseconds(FrameTimeHistory::GetInstance().GetEntry(0).deltaTime));
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(width - 100.0f, maxHeight));
		}

		// TODO
		static float minFPS = std::numeric_limits<float>::max();
		static float maxFPS = std::numeric_limits<float>::min();
		if (FPSCounter::GetInstance().GetFPS() != 0.0f)
		{
			minFPS = std::min(minFPS, FPSCounter::GetInstance().GetFPS());
			maxFPS = std::max(maxFPS, FPSCounter::GetInstance().GetFPS());

		}
		
		std::vector<float> fpsHistorySorted = fpsHistory;
		std::sort(fpsHistorySorted.begin(), fpsHistorySorted.end());
		int onePercentLowIndex= static_cast<int>(0.01f * fpsHistorySorted.size());
		onePercentLowIndex = std::max(1, onePercentLowIndex);

		const float avg = std::accumulate(fpsHistorySorted.begin(), fpsHistorySorted.begin() + fpsHistorySorted.size(), 0.0f) / static_cast<float>(fpsHistorySorted.size());
		const float low1 = std::accumulate(fpsHistorySorted.begin(), fpsHistorySorted.begin() + onePercentLowIndex, 0.0f) / static_cast<float>(onePercentLowIndex);

		ImGui::Text("Avg: %.2f FPS", avg);
		ImGui::Text("Min: %.2f FPS", minFPS);
		ImGui::Text("Max: %.2f FPS", maxFPS);
		ImGui::Text("1%% Low: %.2f FPS", low1);
		
		// Main body of GPU Metrics Window ends here
		ImGui::End();
	}

	void UserInterface::StartBenchmark(const Benchmark benchmark)
	{
		bShowMainMenuBar = false;
		inputManager.StartBenchmark(benchmark);
	}

	glm::vec4 UserInterface::DeltaTimeToColor(float deltaTime)
	{
		constexpr glm::vec3 colors[] = {
			{0.0f, 0.0f, 1.0f}, // blue
			{0.0f, 1.0f, 0.0f}, // green
			{1.0f, 1.0f, 0.0f}, // yellow
			{1.0f, 0.0f, 0.0f}, // red
		};

		constexpr float dts[] = {
			1.0f / 120.0f, // TODO: MAX_FRAME
			1.0f / 60.0f,
			1.0f / 30.0f,
			1.0f / 15.0f,
		};

		size_t countOfDTS = sizeof(dts) / sizeof(dts[0]);

		if (deltaTime < dts[0])
		{
			return glm::vec4(colors[0], 1.f);
		}

		for (size_t i = 1; i < countOfDTS; ++i)
		{
			if (deltaTime < dts[i])
			{
				const float t = (deltaTime - dts[i - 1]) / (dts[i] - dts[i - 1]);
				return glm::vec4(glm::mix(colors[i - 1], colors[i], t), 1.f);
			}
		}

		return glm::vec4(colors[countOfDTS - 1], 1.f);
	}

	UserInterface::ImGuiWindowUserData* UserInterface::CreateNewWindowUserData(const char* name)
	{
		// Allocate chunk
		const size_t nameLen = strlen(name);
		const size_t chunkSize = sizeof(ImGuiWindowUserData) + nameLen + 1;
		ImGuiWindowUserData* userData = UserDataWindows.alloc_chunk(chunkSize);
		IM_PLACEMENT_NEW(userData) ImGuiWindowUserData();
		memcpy(userData->GetName(), name, nameLen + 1); // Store with zero terminator

		return userData;
	}

	void* UserInterface::UserData_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
	{
		return (void*)CreateNewWindowUserData(name);
	}

	void UserInterface::UserData_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
	{
		ImGuiWindowUserData* userData = static_cast<ImGuiWindowUserData*>(entry);

		char showWindow[6] = {};
		if (sscanf(line, "Show=%5s", showWindow) == 1)
		{
			UserDataWindow[userData->GetName()] = (strncmp(showWindow, "true", 5) == 0);
		}
	}

	void UserInterface::UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
	{
		for (ImGuiWindow* window : ctx->Windows)
		{
			if (UserDataWindow.contains(window->Name))
			{
				buf->appendf("[%s][%s]\n", handler->TypeName, window->Name);
				buf->appendf("Show=%s\n", UserDataWindow[window->Name] ? "true" : "false");
				buf->append("\n");
			}
		}
	}

} // namespace VulkanCore
