#include "GPUDevice.h"

#include <stdexcept>
#include <iostream>
#include <set>

// TODO: delete
#include "SwapChain.h"
#include "Pipeline.h"

namespace VulkanCore {

#ifdef DEBUG
    const bool GPUDevice::bEnableValidationLayers = true;
#else
    const bool GPUDevice::bEnableValidationLayers = false;
#endif

    const std::vector<const char*> GPUDevice::instanceExtensions = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
    };

    const std::vector<const char*> GPUDevice::instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> GPUDevice::deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME
    };

    // TODO: de declarat static in .h
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        // TODO: DOODLE -> REZOLVA
        // std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

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

    GPUDevice::GPUDevice(Window& window) // TODO: [TRY] const Window& window
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface(window);
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
        CreateSyncObjects();
    }

    GPUDevice::~GPUDevice()
    {
        // TODO: DOODLE
        vkDestroyFence(device, computeFence, nullptr);
        vkDestroyFence(device, imageFence, nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (bEnableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

    }

    uint32_t GPUDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VkFormat GPUDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (const VkFormat& format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    void GPUDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        // Create buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create buffer!");
        }

        // Memory allocation
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void GPUDevice::CreateImage(const VkFormat& format, const uint32_t& width, const uint32_t& height, const VkImageTiling& tiling, const VkSampleCountFlagBits& samples, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = samples;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkCommandBuffer GPUDevice::BeginSingleTimeCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void GPUDevice::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // TODO: DOODLE
        if (queue == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to submit to queue");
        }

        // Wait for other work on GPU to finish
        vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);

        // Only reset the fence if we are submitting work
        vkResetFences(device, 1, &computeFence);

        // Submit work
        if (vkQueueSubmit(queue, 1, &submitInfo, computeFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit compute command buffer!");
        }
        
        vkQueueWaitIdle(queue );

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void GPUDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

        VkBufferCopy region = {};
        region.srcOffset = 0;		// optional
        region.dstOffset = 0;		// optional
        region.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);

        EndSingleTimeCommandBuffer(commandBuffer, queue);
    }

    void GPUDevice::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        VkOffset3D imageOffset = {};
        imageOffset.x = 0;
        imageOffset.y = 0;
        imageOffset.z = 0;
        region.imageOffset = imageOffset;

        VkExtent3D imageExtent = {};
        imageExtent.width = width;
        imageExtent.height = height;
        imageExtent.depth = 1;
        region.imageExtent = imageExtent;

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        throw std::runtime_error("DO NOT USE");

        EndSingleTimeCommandBuffer(commandBuffer, graphicsQueue);
    }

    void GPUDevice::CreateInstance()
    {
        if (!glfwVulkanSupported())
        {
            throw std::runtime_error("GLFW: Vulkan not supported!");
        }

#ifdef DEBUG
        ListAvailableInstanceExtensions();
        ListRequiredGLFWInstanceExtensions();
        ListRequiredAppInstanceExtensions();
        ListAvailableInstanceLayers();
        ListRequiredInstanceLayers();
#endif // DEBUG

        if (bEnableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Particle System";
        appInfo.pEngineName = "No Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        const std::vector<const char*>& requiredExtensionNames = GetRequiredExtensionNames();

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
        createInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (bEnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
            createInfo.ppEnabledLayerNames = instanceLayers.data();

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

    void GPUDevice::CreateSurface(Window& window)
    {
        // TODO: pune functia asta aici, nu e nevoie de o metoda in clasa window
        window.CreateWindowSurface(instance, &surface);
    }

    void GPUDevice::PickPhysicalDevice()
    {
#ifdef DEBUG
        ListAvailablePhysicalDevices();
#endif // DEBUG

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

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

#ifdef DEBUG
        ListAvailableDeviceExtensions();
#endif
    }

    void GPUDevice::CreateLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies;
        uniqueQueueFamilies.insert(indices.graphicsAndComputeFamily.value());
        uniqueQueueFamilies.insert(indices.presentFamily.value());

        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Additional features
        VkPhysicalDeviceShaderFloat16Int8Features physicalDeviceShaderFloat16Int8Features = {};
        physicalDeviceShaderFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
        physicalDeviceShaderFloat16Int8Features.pNext = nullptr;
        physicalDeviceShaderFloat16Int8Features.shaderFloat16 = 0;
        physicalDeviceShaderFloat16Int8Features.shaderInt8 = 0;

        VkPhysicalDevice16BitStorageFeatures physicalDevice16BitStorageFeatures = {};
        physicalDevice16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
        physicalDevice16BitStorageFeatures.pNext = &physicalDeviceShaderFloat16Int8Features;
        physicalDevice16BitStorageFeatures.storageBuffer16BitAccess = 0;
        physicalDevice16BitStorageFeatures.uniformAndStorageBuffer16BitAccess = 0;
        physicalDevice16BitStorageFeatures.storagePushConstant16 = 0;
        physicalDevice16BitStorageFeatures.storageInputOutput16 = 0;

        VkPhysicalDevice8BitStorageFeatures physicalDevice8BitStorageFeatures = {};
        physicalDevice8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
        physicalDevice8BitStorageFeatures.pNext = &physicalDevice16BitStorageFeatures;
        physicalDevice8BitStorageFeatures.storageBuffer8BitAccess = 0;
        physicalDevice8BitStorageFeatures.uniformAndStorageBuffer8BitAccess = 0;
        physicalDevice8BitStorageFeatures.storagePushConstant8 = 0;

        VkPhysicalDeviceShaderAtomicInt64Features physicalDeviceShaderAtomicInt64Features = {};
        physicalDeviceShaderAtomicInt64Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
        physicalDeviceShaderAtomicInt64Features.pNext = &physicalDevice8BitStorageFeatures;
        physicalDeviceShaderAtomicInt64Features.shaderBufferInt64Atomics = 0;
        physicalDeviceShaderAtomicInt64Features.shaderSharedInt64Atomics = 0;

        VkPhysicalDeviceVariablePointersFeatures physicalDeviceVariablePointersFeatures = {};
        physicalDeviceVariablePointersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
        physicalDeviceVariablePointersFeatures.pNext = &physicalDeviceShaderAtomicInt64Features;
        physicalDeviceVariablePointersFeatures.variablePointersStorageBuffer = 0;
        physicalDeviceVariablePointersFeatures.variablePointers = 0;

        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT physicalDeviceBufferDeviceAddressFeaturesEXT = {};
        physicalDeviceBufferDeviceAddressFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
        physicalDeviceBufferDeviceAddressFeaturesEXT.pNext = &physicalDeviceVariablePointersFeatures;
        physicalDeviceBufferDeviceAddressFeaturesEXT.bufferDeviceAddress = 0;
        physicalDeviceBufferDeviceAddressFeaturesEXT.bufferDeviceAddressCaptureReplay = 0;
        physicalDeviceBufferDeviceAddressFeaturesEXT.bufferDeviceAddressMultiDevice = 0;

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.pNext = &physicalDeviceBufferDeviceAddressFeaturesEXT;
        physicalDeviceFeatures2.features.robustBufferAccess = VK_TRUE;
        physicalDeviceFeatures2.features.largePoints = VK_TRUE;

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.robustBufferAccess = VK_TRUE;
        deviceFeatures.largePoints = VK_TRUE;
        // deviceFeatures.samplerAnisotropy = VK_TRUE; // TODO: adauga pentru texturi

        // Create Device
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &physicalDeviceFeatures2;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        if (bEnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
            createInfo.ppEnabledLayerNames = instanceLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.graphicsAndComputeFamily.value(), 0, &computeQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void GPUDevice::CreateCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void GPUDevice::CreateSyncObjects()
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(device, &fenceInfo, nullptr, &computeFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute synchronization objects!");
        }

        if (vkCreateFence(device, &fenceInfo, nullptr, &imageFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute synchronization objects!");
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
        // Required GLFW Instance Extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensionNames;
        glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> requiredExtensionNames(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);

        // Required Instance Extensions for enabled features
        requiredExtensionNames.insert(requiredExtensionNames.end(), instanceExtensions.begin(), instanceExtensions.end());

        // Required Instance Extension for debugging
        if (bEnableValidationLayers)
        {
            requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return requiredExtensionNames;
    }

    bool GPUDevice::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::set<std::string> requiredLayers(instanceLayers.begin(), instanceLayers.end());
        for (const VkLayerProperties& layer : availableLayers)
        {
            requiredLayers.erase(layer.layerName);
        }

        return requiredLayers.empty();
    }

    bool GPUDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const VkExtensionProperties& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool GPUDevice::IsDeviceSuitable(VkPhysicalDevice device) // TODO: [TRY] const VkPhysicalDevice& device
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        // TODO: nu stiu daca o sa le folosesc
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        const bool bExtensionsSupported = CheckDeviceExtensionSupport(device);

        bool bSwapChainAdequate = false;
        if (bExtensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            bSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // TODO: include mai multe criterii
        return indices.IsComplete() 
            && bExtensionsSupported && bSwapChainAdequate 
            && deviceFeatures.samplerAnisotropy
            && deviceFeatures.largePoints;
    }

    QueueFamilyIndices GPUDevice::FindQueueFamilies(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices = {}; // TODO: salveaza asta undeva, de ce sa fac find de de 4 ori?

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.graphicsAndComputeFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            // TODO: add logic to explicitly prefer a physical device that supports drawing and presentation in the same queue for improved performance

            if (indices.IsComplete())
            {
                break;
            }

            ++i;
        }

        return indices;
    }

    SwapChainSupportDetails GPUDevice::QuerySwapChainSupport(VkPhysicalDevice device) const // TODO: [TRY] const VkPhysicalDevice& device + return const SwapChainSupportDetails?
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

#if DEBUG
    void GPUDevice::ListAvailableInstanceExtensions() const
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        std::cout << "Available Instance Extensions:\n";
        for (const VkExtensionProperties& extension : availableExtensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListRequiredGLFWInstanceExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensionsNames;
        glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::cout << "Required GLFW Instance Extensions:\n";
        for (size_t i = 0; i < glfwExtensionCount; ++i) {
            std::cout << '\t' << glfwExtensionsNames[i] << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListRequiredAppInstanceExtensions() const
    {
        std::cout << "Required App Instance Extensions:\n";
        for (size_t i = 0; i < instanceExtensions.size(); ++i)
        {
            std::cout << '\t' << instanceExtensions[i] << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailableInstanceLayers() const
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "Available Instance Layers:\n";
        for (const VkLayerProperties& Layer : availableLayers) {
            std::cout << '\t' << Layer.layerName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListRequiredInstanceLayers() const
    {
        std::cout << "Required App Instance Layers:\n";
        if (bEnableValidationLayers)
        {
            for (size_t i = 0; i < instanceLayers.size(); ++i)
            {
                std::cout << '\t' << instanceLayers[i] << '\n';
            }
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailableDeviceExtensions() const
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::cout << "Available Device Extensions:\n";
        for (const auto& extension : availableExtensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << '\n';
    }

    void GPUDevice::ListAvailablePhysicalDevices() const
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        uint32_t counter = 0;
        std::cout << "Physical devices found: " << deviceCount << '\n';
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

        std::cout << '\n';
    }
#endif // DEBUG

} // namespace VulkanCore
