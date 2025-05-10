#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"
#include "Renderer.h"
#include "Descriptor.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace VulkanCore {

	class UserInterface
	{
	public:
		// Windows User Data saved in imgui.ini file
		// Because we never destroy or rename ImGuiWindowUserData, we can store the names in a separate buffer easily.
		// (this is designed to be stored in a ImChunkStream buffer, with the variable-length Name following our structure)
		struct ImGuiWindowUserData
		{
			bool show;

			ImGuiWindowUserData() { memset(this, 0, sizeof(*this)); }
			char* GetName() { return (char*)(this + 1); }
		};

		// ImGuiWindowUserData .ini settings entries
		static ImChunkStream<ImGuiWindowUserData>  UserDataWindows;

		// Constructors
		UserInterface(Window& window, GPUDevice& device, Renderer& renderer);

		// Destructor
		~UserInterface();

		// Not copyable
		UserInterface(const UserInterface&) = delete;
		UserInterface& operator = (const UserInterface&) = delete;

		// Not moveable
		UserInterface(UserInterface&&) = delete;
		UserInterface& operator = (UserInterface&&) = delete;

		void Update();
		void Draw(VkCommandBuffer commandBuffer);

		bool GetIsUIFocused() const;

	private:
		// TODO: DELETE? pentru a testa cat de mult duce GPU-ul
		static const uint32_t MAX_PARTICLE_MULTIPLIER;
		static std::unordered_map<std::string, bool> UserDataWindow;

		Window& window;
		GPUDevice& device;
		Renderer& renderer;

		std::unique_ptr<DescriptorPool> imGuiDescriptorPool;

		bool bShowMainMenuBar;
		bool bShowProgressBar;

		uint32_t particleCount;
		glm::vec4 baseColor;

#ifdef DEBUG
		static void CheckImGuiVulkanResult(VkResult err);
#endif // DEBUG

		static void HelpMarker(const std::string desc);

		static glm::vec4 DeltaTimeToColor(float deltaTime);

		static ImGuiWindowUserData* FindWindowUserDataByID(ImGuiID id);
		static ImGuiWindowUserData* CreateNewWindowUserData(const char* name);

		static void* UserData_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
		static void UserData_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
		static void UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

		void CreateDescriptorPool();
		void SetupImGui();

		void BeginRenderPass(VkCommandBuffer commandBuffer);
		void EndRenderPass(VkCommandBuffer commandBuffer);

		void DrawImGui();

		void ShowMainMenuBar();
		void ShowSettingsWindow();
		void ShowGPUWindow();
		void ShowCPUWindow();
		void ShowRAMWindow();
	};

} // namespace VulkanCore
