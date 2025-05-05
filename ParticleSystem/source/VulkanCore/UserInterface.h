#pragma once

#include <memory>
#include <vector>

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"
#include "Renderer.h"
#include "Descriptor.h"

namespace VulkanCore {

	class UserInterface
	{
	public:
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

		Window& window;
		GPUDevice& device;
		Renderer& renderer;

		std::unique_ptr<DescriptorPool> imGuiDescriptorPool;

		bool bShowMainMenuBar;
		bool bShowSettingsWindow;
		bool bShowGPUWindow;
		bool bShowCPUWindow;
		bool bShowRAMWindow;
		bool bShowProgressBar;

		uint32_t particleCount;
		glm::vec4 baseColor;

#ifdef DEBUG
		static void CheckImGuiVulkanResult(VkResult err);
#endif // DEBUG

		static void HelpMarker(const std::string desc);

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
