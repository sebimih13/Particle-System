#pragma once

#include <vector>
#include <optional>

// TODO: sau Forward Declarations
#include "Window.h"

namespace VulkanCore {

    struct QueueFamilyIndices final
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        inline bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapChainSupportDetails final
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class GPUDevice final
    {
    public:
        // Constructor
        GPUDevice(Window& window);

        // Destructor
        ~GPUDevice();

        // Not copyable
        GPUDevice(const GPUDevice&) = delete;
        GPUDevice& operator = (const GPUDevice&) = delete;

        // Not moveable
        GPUDevice(GPUDevice&&) = delete;
        GPUDevice& operator = (GPUDevice&&) = delete;

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

        // Single time command buffer
        VkCommandBuffer BeginSingleTimeCommandBuffer();
        void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);

        // Copy Buffer
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        // Getters
        inline VkInstance GetInstance() const { return instance; }      // TODO: return const &
        inline VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }    // TODO: return const &
        inline VkSurfaceKHR GetSurface() const { return surface; }      // TODO: return const &
        inline VkDevice GetVKDevice() const { return device; }          // TODO: return const &
        inline SwapChainSupportDetails GetSwapChainSupport() const { return QuerySwapChainSupport(physicalDevice); }    // TODO: return const &
        inline QueueFamilyIndices GetPhysicalQueueFamilies() const { return FindQueueFamilies(physicalDevice); }        // TODO: return const &
        inline VkCommandPool GetCommandPool() const { return commandPool; }     // TODO: return const &
        inline VkQueue GetGraphicsQueue() const { return graphicsQueue; }       // TODO: return const &
        inline VkQueue GetPresentQueue() const { return presentQueue; }         // TODO: return const &

    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkCommandPool commandPool;

#ifdef DEBUG
        const bool bEnableValidationLayers = true;
#else
        const bool bEnableValidationLayers = false;
#endif

        static const std::vector<const char*> validationLayers;
        static const std::vector<const char*> deviceExtensions;

        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface(Window& window);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        std::vector<const char*> GetRequiredExtensionNames();
        bool CheckValidationLayerSupport();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        bool IsDeviceSuitable(VkPhysicalDevice device);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        // DEBUG
        void ListAvailableExtensions() const;
        void ListRequiredGLFWExtensions() const;
        void ListAvailableValidationLayers() const;
        void ListAvailablePhysicalDevices() const;
    };

} // namespace VulkanCore
