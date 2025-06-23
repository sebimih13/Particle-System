#include "SwapChain.h"

#include <limits>
#include <algorithm>
#include <array>
#include <stdexcept>

namespace VulkanCore {

	SwapChain::SwapChain(GPUDevice& device, const Window& window)
        : device(device)
        , window(window)
        , currentFrameIndex(0)
	{
        CreateSwapChain();
        CreateRenderPass();
        CreateImageViews();
        // CreateDepthResources(); // [PARTICLE-SYSTEM]
        CreateIntermediaryImageViews();
        CreateFramebuffers();
        CreateSyncObjects();

        // ImGui
        CreateImGuiRenderPass();
        CreateImGuiFramebuffers();
	}

	SwapChain::~SwapChain()
	{
        // cleanup synchronization objects
        vkDestroySemaphore(device.GetVKDevice(), imageSemaphore, nullptr);

        // cleanup framebuffers
        for (VkFramebuffer framebuffer : imGuiFramebuffers)
        {
            vkDestroyFramebuffer(device.GetVKDevice(), framebuffer, nullptr);
        }
        
        for (VkFramebuffer framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(device.GetVKDevice(), framebuffer, nullptr);
        }

        // cleanup render pass
        vkDestroyRenderPass(device.GetVKDevice(), imGuiRenderPass, nullptr);
        vkDestroyRenderPass(device.GetVKDevice(), renderPass, nullptr);

        // cleanup intermediary images
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            vkDestroyImageView(device.GetVKDevice(), intermediaryImageViews[i], nullptr);
            vkDestroyImage(device.GetVKDevice(), intermediaryImages[i], nullptr);
            vkFreeMemory(device.GetVKDevice(), intermediaryImageMemories[i], nullptr);
        }

        // cleanup image views
        for (VkImageView imageView : swapChainImageViews)
        {
            vkDestroyImageView(device.GetVKDevice(), imageView, nullptr);
        }

        // cleanup swap chain
        vkDestroySwapchainKHR(device.GetVKDevice(), swapChain, nullptr);
	}

    VkResult SwapChain::AcquireNextImage(uint32_t* imageIndex)
    {
        // vkWaitForFences(device.GetVKDevice(), 1, &inFlightFences[currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        return vkAcquireNextImageKHR(device.GetVKDevice(), swapChain, std::numeric_limits<uint64_t>::max(), imageSemaphore, VK_NULL_HANDLE, imageIndex);
    }

    void SwapChain::SubmitComputeCommandBuffer(const VkCommandBuffer* buffer)
    {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        // Wait for other work on GPU to finish
        vkWaitForFences(device.GetVKDevice(), 1, &device.GetComputeFence(), VK_TRUE, UINT64_MAX);

        // Only reset the fence if we are submitting work
        vkResetFences(device.GetVKDevice(), 1, &device.GetComputeFence());

        // Submit work
        if (vkQueueSubmit(device.GetComputeQueue(), 1, &submitInfo, device.GetComputeFence()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit compute command buffer!");
        }
    }

    VkResult SwapChain::SubmitCommandBuffer(const VkCommandBuffer* buffer, uint32_t* imageIndex)
    {
        // VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrameIndex], imageAvailableSemaphores[currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        // Wait for other work on GPU to finish
        vkWaitForFences(device.GetVKDevice(), 1, &device.GetComputeFence(), VK_TRUE, UINT64_MAX);

        // Only reset the fence if we are submitting work
        vkResetFences(device.GetVKDevice(), 1, &device.GetImageFence());

        if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = { swapChain };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = imageIndex;
        presentInfo.pResults = nullptr;     // optional

        return vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);
    }

    void SwapChain::SubmitSyncNewFrameCommandBuffer(const VkCommandBuffer* buffer)
    {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        // Submit work
        if (vkQueueSubmit(device.GetComputeQueue(), 1, &submitInfo, device.GetImageFence()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit compute command buffer!");
        }

        // Wait for other work on GPU to finish
        vkWaitForFences(device.GetVKDevice(), 1, &device.GetImageFence(), VK_TRUE, UINT64_MAX);
    }

    void SwapChain::AdvanceFrameIndex()
    {
        currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;        
    }

    void SwapChain::CreateSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = device.GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = device.GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device.GetPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsAndComputeFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;    // This option offers the best performance
            createInfo.queueFamilyIndexCount = 0;       // optional
            createInfo.pQueueFamilyIndices = nullptr;   // optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_FALSE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device.GetVKDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device.GetVKDevice(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.GetVKDevice(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    VkImageView SwapChain::CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectMask) const
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectMask;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device.GetVKDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view!");
        }

        return imageView;
    }

    void SwapChain::CreateImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void SwapChain::CreateIntermediaryImageViews()
    {
        intermediaryImageMemories.resize(swapChainImages.size());
        intermediaryImages.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            device.CreateImage(
                swapChainImageFormat,
                swapChainExtent.width,
                swapChainExtent.height,
                VK_IMAGE_TILING_OPTIMAL,
                VK_SAMPLE_COUNT_8_BIT,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                intermediaryImages[i],
                intermediaryImageMemories[i]
            );
        }

        intermediaryImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            intermediaryImageViews[i] = CreateImageView(intermediaryImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void SwapChain::CreateRenderPass()
    {
        VkAttachmentDescription intermediaryAttachment = {};
        intermediaryAttachment.format = swapChainImageFormat;
        intermediaryAttachment.samples = VK_SAMPLE_COUNT_8_BIT;
        intermediaryAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        intermediaryAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        intermediaryAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        intermediaryAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        intermediaryAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        intermediaryAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // Color Attachment Ref for Subpass = intermediaryAttachment
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Resolve Attachment Ref for Subpass = colorAttachment
        VkAttachmentReference resolveAttachmentRef = {};
        resolveAttachmentRef.attachment = 1;
        resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pResolveAttachments = &resolveAttachmentRef;

        std::array<VkAttachmentDescription, 2> attachments = { intermediaryAttachment, colorAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(device.GetVKDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    void SwapChain::CreateFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = {
                intermediaryImageViews[i],
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.GetVKDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void SwapChain::CreateSyncObjects()
    {
        //imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        //renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        //computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

        //inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device.GetVKDevice(), &semaphoreInfo, nullptr, &imageSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute synchronization objects for a frame!");
        }

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        //{
        //    if (vkCreateSemaphore(device.GetVKDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
        //        || vkCreateSemaphore(device.GetVKDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
        //        || vkCreateFence(device.GetVKDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        //    {
        //        throw std::runtime_error("Failed to create synchronization objects for a frame!");
        //    }

        //    if (vkCreateSemaphore(device.GetVKDevice(), &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS)
        //    {
        //        throw std::runtime_error("Failed to create compute synchronization objects for a frame!");
        //    }
        //}
    }

    void SwapChain::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();
        device.CreateImage(
            depthFormat,
            swapChainExtent.width,
            swapChainExtent.height,
            VK_IMAGE_TILING_OPTIMAL,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage,
            depthImageMemory
        );

        depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void SwapChain::CreateImGuiRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 1> attachments = {
            colorAttachment
        };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device.GetVKDevice(), &renderPassInfo, nullptr, &imGuiRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui render pass!");
        }
    }

    void SwapChain::CreateImGuiFramebuffers()
    {
        imGuiFramebuffers.resize(swapChainImages.size());

        for (size_t i = 0; i < imGuiFramebuffers.size(); ++i)
        {
            std::array<VkImageView, 1> attachments = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = imGuiRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.GetVKDevice(), &framebufferInfo, nullptr, &imGuiFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create ImGui framebuffer!");
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        // [DEBUG] list all available present modes

        for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
        {
            // Integrated GPU -> VK_PRESENT_MODE_IMMEDIATE_KHR
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                return availablePresentMode;
            }
        }

        // VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent;
        actualExtent.width = static_cast<uint32_t>(window.GetWidth());
        actualExtent.height = static_cast<uint32_t>(window.GetHeight());

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    VkFormat SwapChain::FindDepthFormat() const
    {
        return device.FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }
            , VK_IMAGE_TILING_OPTIMAL
            , VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool SwapChain::HasStencilComponent(VkFormat format) const
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

} // namespace VulkanCore
