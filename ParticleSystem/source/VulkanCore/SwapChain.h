#pragma once

#include <vector>

// TODO: asta sau alt Window / GPUDevice
#include "Window.h"
#include "GPUDevice.h"

namespace VulkanCore {

	class SwapChain final
	{
	public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        
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

        VkResult AcquireNextImage(uint32_t* imageIndex);
        void AcquireNextCompute();

        void SubmitComputeCommandBuffer(const VkCommandBuffer* buffer);
        VkResult SubmitCommandBuffer(const VkCommandBuffer* buffer, uint32_t* imageIndex);

        void AdvanceFrameIndex();

        // Getters
        VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
        VkRenderPass GetRenderPass() const { return renderPass; }
        VkFramebuffer GetSwapChainFramebuffer(const size_t& index) const { return swapChainFramebuffers[index]; }
        uint32_t GetCurrentFrameIndex() const { return currentFrameIndex; }

	private:
        GPUDevice& device; // TODO: const?
        const Window& window;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;

        // Sync Objects
        uint32_t currentFrameIndex;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkSemaphore> computeFinishedSemaphores;

        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> computeInFlightFences;

        // depth image and view
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        void CreateSwapChain();
        VkImageView CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectFlags) const;
        void CreateImageViews();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();
        void CreateDepthResources();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        VkFormat FindDepthFormat() const;
        bool HasStencilComponent(VkFormat format) const; // TODO: use
	};

} // namespace VulkanCore
