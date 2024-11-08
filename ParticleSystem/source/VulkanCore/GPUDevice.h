#pragma once

// TODO: sau Forward Declarations
#include "Window.h"

#include <vector>

namespace VulkanCore {

    class GPUDevice
    {
    public:
        // Constructor
        GPUDevice();

        // Destructor
        ~GPUDevice();

        // Not copyable
        GPUDevice(const GPUDevice&) = delete;
        GPUDevice& operator = (const GPUDevice&) = delete;

        // Not moveable
        GPUDevice(Window&&) = delete;
        GPUDevice& operator = (GPUDevice&&) = delete;

    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

#ifdef DEBUG
        const bool bEnableValidationLayers = true;
#else
        const bool bEnableValidationLayers = false;
#endif

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        void CreateInstance();
        void SetupDebugMessenger();
        void PickPhysicalDevice();

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        std::vector<const char*> GetRequiredExtensionNames();
        bool CheckValidationLayerSupport();

        void ListAvailableExtensions();
        void ListRequiredGLFWExtensions();
        void ListAvailableValidationLayers();
    };

} // namespace VulkanCore
