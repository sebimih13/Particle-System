#include "Texture.h"

#include "stb_image.h"

#include <stdexcept>
#include <cstring>

namespace VulkanCore {

	Texture::Texture(GPUDevice& device, const std::string& filePath)
		: device(device)
	{
		CreateTextureImage(filePath);
		CreateTextureImageView();
		CreateTextureSampler();
	}

	Texture::~Texture()
	{
		vkDestroySampler(device.GetVKDevice(), textureSampler, nullptr);
		vkDestroyImageView(device.GetVKDevice(), textureImageView, nullptr);
		vkDestroyImage(device.GetVKDevice(), textureImage, nullptr);
		vkFreeMemory(device.GetVKDevice(), textureImageMemory, nullptr);
	}

	void Texture::CreateTextureImage(const std::string& filePath)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		// Create staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.CreateBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		// Filling staging buffer
		void* data;
		vkMapMemory(device.GetVKDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			std::memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.GetVKDevice(), stagingBufferMemory);

		// Clear the original pixel array
		stbi_image_free(pixels);

		// Create Image
		device.CreateImage(
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage,
			textureImageMemory
		);

		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			device.CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

	void Texture::CreateTextureImageView()
	{
		// TODO: use SwapChain::CreateImageView() function

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = textureImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.GetVKDevice(), &createInfo, nullptr, &textureImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view!");
		}
	}

	void Texture::CreateTextureSampler()
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(device.GetVKDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void Texture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = device.BeginSingleTimeCommandBuffer();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		device.EndSingleTimeCommandBuffer(commandBuffer);
	}

} // namespace VulkanCore
