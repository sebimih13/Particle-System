#pragma once

// TODO: asta sau alt Window / GPUDevice
#include "Window.h"
#include "GPUDevice.h"

#include <vector>

namespace VulkanCore {

	class SwapChain
	{
	public:
        // Constructor
        SwapChain(GPUDevice& device, const Window& window);

        // Destructor
        ~SwapChain();

        // Not copyable
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator = (const SwapChain&) = delete;

        // Not moveable
        SwapChain(SwapChain&&) = delete;
        SwapChain& operator = (SwapChain&&) = delete;

	private:
        GPUDevice& device; // TODO: const?
        const Window& window;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        void CreateSwapChain();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	};

} // namespace VulkanCore