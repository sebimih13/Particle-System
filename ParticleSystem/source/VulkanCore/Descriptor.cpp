#include "Descriptor.h"

#include <stdexcept>

namespace VulkanCore {

	DescriptorSetLayout::Builder::Builder(GPUDevice& device)
		: device(device)
	{

	}

	DescriptorSetLayout::Builder::~Builder()
	{

	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const
	{
		return std::make_unique<DescriptorSetLayout>(device, bindings);
	}

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count)
	{
		// TODO: assert() - bindings[binding]

		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		layoutBinding.pImmutableSamplers = nullptr;		// optional

		// save binding
		bindings[binding] = layoutBinding;

		return *this;
	}

	DescriptorSetLayout::DescriptorSetLayout(GPUDevice& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings)
		: device(device)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		for (const auto& kv : bindings)
		{
			layoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(device.GetVKDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(device.GetVKDevice(), descriptorSetLayout, nullptr);
	}

	DescriptorPool::Builder::Builder(GPUDevice& device)
		: device(device)
		, maxSets(0)
	{

	}

	DescriptorPool::Builder::~Builder()
	{

	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const
	{
		return std::make_unique<DescriptorPool>(device, poolSizes, maxSets);
	}

	DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType type, uint32_t count)
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = type;
		poolSize.descriptorCount = count;

		poolSizes.push_back(poolSize);

		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count)
	{
		maxSets = count;

		return *this;
	}

	DescriptorPool::DescriptorPool(GPUDevice& device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
		: device(device)
	{
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;
		// poolInfo.flags = poolFlags;		// TODO: add

		if (vkCreateDescriptorPool(device.GetVKDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(device.GetVKDevice(), descriptorPool, nullptr);
	}

	bool DescriptorPool::AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(device.GetVKDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

} // namespace VulkanCore