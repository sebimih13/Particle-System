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

		void Draw(VkCommandBuffer commandBuffer);

		bool GetIsUIFocused() const;

	private:
		Window& window;
		GPUDevice& device;
		Renderer& renderer;

		std::unique_ptr<DescriptorPool> imGuiDescriptorPool;

#ifdef DEBUG
		static void CheckImGuiVulkanResult(VkResult err);
#endif // DEBUG

		void CreateDescriptorPool();
		void SetupImGui();

		void BeginRenderPass(VkCommandBuffer commandBuffer);
		void EndRenderPass(VkCommandBuffer commandBuffer);
	};

} // namespace VulkanCore
