#pragma once

#include <vector>
#include <optional>

// TODO: sau Forward Declarations
#include "Window.h"

namespace VulkanCore {

    struct QueueFamilyIndices final
    {
        std::optional<uint32_t> graphicsAndComputeFamily;
        std::optional<uint32_t> presentFamily;

        inline bool IsComplete() { return graphicsAndComputeFamily.has_value() && presentFamily.has_value(); }
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

        // Utils
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CreateImage(const VkFormat& format, const uint32_t& width, const uint32_t& height, const VkImageTiling& tiling, const VkSampleCountFlagBits& samples, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkImage& image, VkDeviceMemory& imageMemory);

        // Single time command buffer
        VkCommandBuffer BeginSingleTimeCommandBuffer();
        void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

        // Copy Buffer
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        // Getters
        inline VkInstance GetInstance() const { return instance; }
        inline VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
        inline VkSurfaceKHR GetSurface() const { return surface; }
        inline VkDevice GetVKDevice() const { return device; }
        inline SwapChainSupportDetails GetSwapChainSupport() const { return QuerySwapChainSupport(physicalDevice); }    // TODO: return const &
        inline QueueFamilyIndices GetPhysicalQueueFamilies() const { return FindQueueFamilies(physicalDevice); }        // TODO: return const &
        inline VkCommandPool GetCommandPool() const { return commandPool; }
        inline VkQueue GetGraphicsQueue() const { return graphicsQueue; }
        inline VkQueue GetComputeQueue() const { return computeQueue; }
        inline VkQueue GetPresentQueue() const { return presentQueue; }

        inline const VkFence& GetComputeFence() const { return computeFence; }
        inline const VkFence& GetImageFence() const { return imageFence; }

        inline const std::string& GetName() const { return name; }

    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // TODO: de pus in Constructor
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue computeQueue;
        VkQueue presentQueue;

        VkCommandPool commandPool;

        VkFence imageFence;
        VkFence computeFence;

        std::string name;

        static const bool bEnableValidationLayers;
        static const std::vector<const char*> instanceExtensions;
        static const std::vector<const char*> instanceLayers;
        static const std::vector<const char*> deviceExtensions;

        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface(Window& window);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();
        void CreateSyncObjects();

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        std::vector<const char*> GetRequiredExtensionNames();
        bool CheckValidationLayerSupport();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
        bool IsDeviceSuitable(VkPhysicalDevice device) const;

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

#ifdef DEBUG
        // Instance
        void ListAvailableInstanceExtensions() const;
        void ListRequiredGLFWInstanceExtensions() const;
        void ListRequiredAppInstanceExtensions() const;
        void ListAvailableInstanceLayers() const;
        void ListRequiredInstanceLayers() const;

        // Device
        void ListAvailableDeviceExtensions() const;
        void ListAvailablePhysicalDevices() const;
#endif // DEBUG
    };

} // namespace VulkanCore
