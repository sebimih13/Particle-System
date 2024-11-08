#include "GPUDevice.h"

#include <stdexcept>
#include <iostream>

namespace VulkanCore {

    // TODO: de declarat static in .h
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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
        PickPhysicalDevice();
    }

    GPUDevice::~GPUDevice()
    {
        if (bEnableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);
    }

    void GPUDevice::CreateInstance()
    {
        if (bEnableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan";                    // TODO: change
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // TODO: schimba la fiecare milestone
        appInfo.pEngineName = "No Engine";                      // TODO: change?
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // TODO: schimba la fiecare milestone
        appInfo.apiVersion = VK_API_VERSION_1_0;

#ifdef DEBUG
        ListAvailableExtensions();
        ListRequiredGLFWExtensions();
        ListAvailableValidationLayers();
#endif // DEBUG

        const std::vector<const char*>& requiredExtensionNames = GetRequiredExtensionNames();

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
        createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (bEnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
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

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void GPUDevice::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

#ifdef DEBUG
        ListAvailablePhysicalDevices();
#endif // DEBUG


        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    void GPUDevice::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;
    }

    std::vector<const char*> GPUDevice::GetRequiredExtensionNames()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensionNames;
        glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensionNames(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);

        if (bEnableValidationLayers)
        {
            extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensionNames;
    }

    bool GPUDevice::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const std::string& layerName : validationLayers)
        {
            bool bFound = false;

            for (const VkLayerProperties& layerProperties : availableLayers)
            {
                if (layerName == layerProperties.layerName)
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

    bool GPUDevice::IsDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // TODO: include mai multe criterii
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
    }

    void GPUDevice::ListAvailableExtensions()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        std::cout << "Available extensions:\n";
        for (const auto& extension : availableExtensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListRequiredGLFWExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensionsNames;
        glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::cout << "Required GLFW extensions:\n";
        for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
            std::cout << '\t' << glfwExtensionsNames[i] << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailableValidationLayers()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "Available validation layers:\n";
        for (const auto& Layer : availableLayers) {
            std::cout << '\t' << Layer.layerName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailablePhysicalDevices()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        uint32_t counter = 0;
        std::cout << "Devices found: " << deviceCount << '\n';
        for (const VkPhysicalDevice& device : devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            std::cout << ++counter << ". " << deviceProperties.deviceName << '\n';

            switch (deviceProperties.deviceType)
            {
                case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: 
                    std::cout << "\tINTEGRATED_GPU\n";
                    break;

                case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    std::cout << "\tDISCRETE_GPU\n";
                    break;
                
                default:
                    std::cout << "\tOTHER\n";
                    break;
            }
        }
    }

} // namespace VulkanCore
