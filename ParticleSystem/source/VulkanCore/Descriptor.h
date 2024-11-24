#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <memory>
#include <unordered_map>

// TODO: Forward Declaration
#include "GPUDevice.h"

namespace VulkanCore {

	class DescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			// TODO
			Builder(GPUDevice& device);
			~Builder();

			std::unique_ptr<DescriptorSetLayout> Build() const;

			Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count);

		private:
			GPUDevice& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
			VkDescriptorPoolCreateFlags descriptorPoolCreateFlags;
		};

		// TODO: de pus in private si de facut clasa Builder friend
		// Constructors
		DescriptorSetLayout(GPUDevice& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);

		// Destructor
		~DescriptorSetLayout();

		// Not copyable
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator = (const DescriptorSetLayout&) = delete;

		// Not moveable
		DescriptorSetLayout(DescriptorSetLayout&&) = delete;
		DescriptorSetLayout& operator = (DescriptorSetLayout&&) = delete;

		// Getters
		VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		GPUDevice& device;

		VkDescriptorSetLayout descriptorSetLayout;
	};

	class DescriptorPool
	{
	public:
		class Builder
		{
		public:
			// Constructors
			Builder(GPUDevice& device);

			// Destructor
			~Builder();

			std::unique_ptr<DescriptorPool> Build() const;

			Builder& AddPoolSize(VkDescriptorType type, uint32_t count);
			Builder& SetMaxSets(uint32_t count);
			Builder& AddFlags(VkDescriptorPoolCreateFlags flags);

		private:
			GPUDevice& device;
			std::vector<VkDescriptorPoolSize> poolSizes;
			uint32_t maxSets;
			VkDescriptorPoolCreateFlags descriptorPoolCreateFlags;
		};

		// Constructors
		DescriptorPool(GPUDevice& device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets, VkDescriptorPoolCreateFlags descriptorPoolCreateFlags);

		// Destructor
		~DescriptorPool();

		// Not copyable
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator = (const DescriptorPool&) = delete;

		// Not moveable
		DescriptorPool(DescriptorPool&&) = delete;
		DescriptorPool& operator = (DescriptorPool&&) = delete;

		bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

		// Getters
		inline VkDescriptorPool GetDescriptorPool() const { return descriptorPool; }

	private:
		GPUDevice& device;
		
		VkDescriptorPool descriptorPool;
	};

} // namespace VulkanCore
