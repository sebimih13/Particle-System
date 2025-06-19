#pragma once

#include <GLFW/glfw3.h>

#include <string>

#include "GPUDevice.h"

namespace VulkanCore {

	class Texture
	{
	public:
		// Builder
		Texture(GPUDevice& device, const std::string& filePath);

		// Destructor
		~Texture();

		// Getters
		inline VkImageView GetTextureImageView() { return textureImageView; }
		inline VkSampler GetTextureSampler() { return textureSampler; }

	private:
		GPUDevice& device;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

		void CreateTextureImage(const std::string& filePath);
		void CreateTextureImageView();
		void CreateTextureSampler();

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	};

} // namespace VulkanCore
