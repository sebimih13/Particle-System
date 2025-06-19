#pragma once

#include <vector>

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
        // void AcquireNextCompute();

        void SubmitComputeCommandBuffer(const VkCommandBuffer* buffer);
        VkResult SubmitCommandBuffer(const VkCommandBuffer* buffer, uint32_t* imageIndex);
        void SubmitSyncNewFrameCommandBuffer(const VkCommandBuffer* buffer);

        void AdvanceFrameIndex();

        // Getters
        inline VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
        inline VkRenderPass GetRenderPass() const { return renderPass; }
        inline VkRenderPass GetImGuiRenderPass() const { return imGuiRenderPass; }
        inline VkFramebuffer GetSwapChainFramebuffer(const size_t& index) const { return swapChainFramebuffers[index]; }
        inline VkFramebuffer GetImGuiFramebuffer(const size_t& index) const { return imGuiFramebuffers[index]; }
        inline uint32_t GetCurrentFrameIndex() const { return currentFrameIndex; }
        
        inline VkImage GetIntermediaryImage(const size_t& index) const { return intermediaryImages[index]; }
        inline VkImage GetSwapchainImage(const size_t& index) const { return swapChainImages[index]; }
        inline VkImageView GetSwapChainImageView(const size_t& index) const { return swapChainImageViews[index]; }

	private:
        GPUDevice& device;
        const Window& window;

        VkRenderPass renderPass;

        VkSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        // Color Images
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        // Intermediary Images for multi-sampling
        std::vector<VkDeviceMemory> intermediaryImageMemories;
        std::vector<VkImage> intermediaryImages;
        std::vector<VkImageView> intermediaryImageViews;

        std::vector<VkFramebuffer> swapChainFramebuffers;

        // Sync Objects
        uint32_t currentFrameIndex;
        //std::vector<VkSemaphore> imageAvailableSemaphores;
        //std::vector<VkSemaphore> renderFinishedSemaphores;
        //std::vector<VkSemaphore> computeFinishedSemaphores;

        //std::vector<VkFence> inFlightFences;

        VkSemaphore imageSemaphore;

        // ImGui
        VkRenderPass imGuiRenderPass;
        std::vector<VkFramebuffer> imGuiFramebuffers;

        // depth image and view
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        void CreateSwapChain();
        VkImageView CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectMask) const;
        void CreateImageViews();
        void CreateIntermediaryImageViews();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();
        void CreateDepthResources();

        // ImGui
        void CreateImGuiRenderPass();
        void CreateImGuiFramebuffers();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        VkFormat FindDepthFormat() const;
        bool HasStencilComponent(VkFormat format) const;
	};

} // namespace VulkanCore
