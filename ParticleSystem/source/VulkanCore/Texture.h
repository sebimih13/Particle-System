#pragma once

// TODO: remove
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

// TODO: Forward Declaration
#include "GPUDevice.h"

namespace VulkanCore {

	class Texture
	{
	public:
		// Builder
		Texture(GPUDevice& device);

		// Destructor
		~Texture();

	private:
		GPUDevice& device;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;

		void CreateTextureImage();
		void CreateImage(const uint32_t& width, const uint32_t& height, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& properties, VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	};

} // namespace VulkanCore
