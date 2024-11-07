#include "GPUDevice.h"

#include <stdexcept>
#include <iostream>

namespace VulkanCore {

    // TODO: de declarat static in .h
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    // TODO: de declarat static in .h
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    // TODO: de declarat in static .h
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    GPUDevice::GPUDevice()
    {
        CreateInstance();
        SetupDebugMessenger();
    }

    GPUDevice::~GPUDevice()
    {
        if (bEnableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(Instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(Instance, nullptr);
    }

    void GPUDevice::CreateInstance()
    {
        if (bEnableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo AppInfo = {};
        AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        AppInfo.pApplicationName = "Vulkan";                    // TODO: change
        AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // TODO: schimba la fiecare milestone
        AppInfo.pEngineName = "No Engine";                      // TODO: change?
        AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // TODO: schimba la fiecare milestone
        AppInfo.apiVersion = VK_API_VERSION_1_0;

#ifdef DEBUG
        ListAvailableExtensions();
        ListRequiredGLFWExtensions();
        ListAvailableValidationLayers();
#endif // DEBUG

        const std::vector<const char*>& RequiredExtensionNames = GetRequiredExtensionNames();

        VkInstanceCreateInfo CreateInfo = {};
        CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        CreateInfo.pApplicationInfo = &AppInfo;
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(RequiredExtensionNames.size());
        CreateInfo.ppEnabledExtensionNames = RequiredExtensionNames.data();

        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {};
        if (bEnableValidationLayers)
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            CreateInfo.ppEnabledLayerNames = ValidationLayers.data();

            PopulateDebugMessengerCreateInfo(DebugCreateInfo);
            CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
        }
        else
        {
            CreateInfo.enabledLayerCount = 0;
            CreateInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    void GPUDevice::SetupDebugMessenger()
    {
        if (!bEnableValidationLayers)
        {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT CreateInfo = {};
        PopulateDebugMessengerCreateInfo(CreateInfo);

        if (CreateDebugUtilsMessengerEXT(Instance, &CreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void GPUDevice::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
    {
        CreateInfo = {};
        CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        CreateInfo.pfnUserCallback = debugCallback;
        CreateInfo.pUserData = nullptr;
    }

    std::vector<const char*> GPUDevice::GetRequiredExtensionNames()
    {
        uint32_t GLFWExtensionCount = 0;
        const char** GLFWExtensionNames;
        GLFWExtensionNames = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

        std::vector<const char*> ExtensionNames(GLFWExtensionNames, GLFWExtensionNames + GLFWExtensionCount);

        if (bEnableValidationLayers)
        {
            ExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return ExtensionNames;
    }

    bool GPUDevice::CheckValidationLayerSupport()
    {
        uint32_t LayerCount;
        vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

        std::vector<VkLayerProperties> AvailableLayers(LayerCount);
        vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

        for (const std::string& LayerName : ValidationLayers)
        {
            bool bFound = false;

            for (const VkLayerProperties& LayerProperties : AvailableLayers)
            {
                if (LayerName == LayerProperties.layerName)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                return false;
            }
        }

        return true;
    }

    void GPUDevice::ListAvailableExtensions()
    {
        uint32_t ExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

        std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, AvailableExtensions.data());

        std::cout << "Available extensions:\n";
        for (const auto& Extension : AvailableExtensions) {
            std::cout << '\t' << Extension.extensionName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListRequiredGLFWExtensions()
    {
        uint32_t GLFWExtensionCount = 0;
        const char** GLFWExtensionsNames;
        GLFWExtensionsNames = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

        std::cout << "Required GLFW extensions:\n";
        for (int i = 0; i < GLFWExtensionCount; ++i) {
            std::cout << '\t' << GLFWExtensionsNames[i] << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailableValidationLayers()
    {
        uint32_t LayerCount;
        vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

        std::vector<VkLayerProperties> AvailableLayers(LayerCount);
        vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

        std::cout << "Available validation layers:\n";
        for (const auto& Layer : AvailableLayers) {
            std::cout << '\t' << Layer.layerName << '\n';
        }
        std::cout << '\n';
    }

} // namespace VulkanCore
