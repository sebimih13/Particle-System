#include "Pipeline.h"

#include <fstream>
#include <iostream>
#include <array>

namespace VulkanCore {

	const std::vector<VkDynamicState> Pipeline::dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	Pipeline::Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout, const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
		: device(device)
		, hasComputePipeline(false)
	{
		CreateGraphicsPipeline(renderPass, descriptorSetLayout, bindingDescription, attributeDescription, vertexShaderFilePath, fragmentShaderFilePath);
	}

	Pipeline::Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& descriptorSetLayout, const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& computeShaderFilePath)
		: device(device)
		, hasComputePipeline(true)
	{
		CreateGraphicsPipeline(renderPass, std::nullopt, bindingDescription, attributeDescription, vertexShaderFilePath, fragmentShaderFilePath);
		CreateComputePipeline(descriptorSetLayout, computeShaderFilePath);
	}

	Pipeline::Pipeline(GPUDevice& device, const VkRenderPass& renderPass, const VkDescriptorSetLayout& graphicsDescriptorSetLayout, const VkDescriptorSetLayout& computeDescriptorSetLayou, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, const std::string& computeShaderFilePath)
		: device(device)
		, hasComputePipeline(true)
	{
		CreateGraphicsPipeline(renderPass, graphicsDescriptorSetLayout, std::nullopt, std::nullopt, vertexShaderFilePath, fragmentShaderFilePath);
		CreateComputePipeline(computeDescriptorSetLayou, computeShaderFilePath);
	}

	Pipeline::~Pipeline()
	{
		if (hasComputePipeline)
		{
			vkDestroyPipeline(device.GetVKDevice(), computePipeline, nullptr);
			vkDestroyPipelineLayout(device.GetVKDevice(), computePipelineLayout, nullptr);
		}

		vkDestroyPipeline(device.GetVKDevice(), graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device.GetVKDevice(), pipelineLayout, nullptr);
	}

	void Pipeline::BindComputePipeline(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
	}

	void Pipeline::BindGraphicsPipeline(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	std::vector<char> Pipeline::ReadFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file: " + filePath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		
		return buffer;
	}

	void Pipeline::CreateGraphicsPipeline(const VkRenderPass& renderPass, const std::optional<VkDescriptorSetLayout>& descriptorSetLayout, const std::optional<VkVertexInputBindingDescription>& bindingDescription, const std::optional<std::vector<VkVertexInputAttributeDescription>>& attributeDescription, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
	{
		// Shader Code
		std::vector<char> vertShaderCode = ReadFile(vertexShaderFilePath);
		std::vector<char> fragShaderCode = ReadFile(fragmentShaderFilePath);

		// Shader Modules
		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		// Vertex Shader
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		// Fragment Shader
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		// Shaders Pipeline
		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		// Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (bindingDescription.has_value() && attributeDescription.has_value())
		{
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription.value();
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.value().size());
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.value().data();
		}

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// TODO: TEST -> DYNAMIC STATE
		// Viewport 
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = 1200;
		viewport.height = 800;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// TODO: TEST -> DYNAMIC STATE
		// Scissor
		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = 0x7FFFFFFF;
		scissor.extent.height = 0x7FFFFFFF;

		VkPipelineViewportStateCreateInfo viewportStateInfo = {};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewport;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissor;

		// TODO: DOODLE : REINTEGRATE DYNAMIC STATES
		// Viewport + Scissors as Dynamic States
		//VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		//dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		//dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		//dynamicStateInfo.pDynamicStates = dynamicStates.data();

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rasterizationInfo.depthBiasClamp = 0.0f;
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;
		rasterizationInfo.lineWidth = 1.0f;

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.minSampleShading = 0.0f;
		multisamplingInfo.pSampleMask = nullptr;				// optional
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE;		// optional
		multisamplingInfo.alphaToOneEnable = VK_FALSE;			// optional

		// Depth
		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.depthTestEnable = VK_FALSE;
		depthStencilStateInfo.depthWriteEnable = VK_FALSE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 0.0f;

		// Color Blend
		VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo = {};
		colorBlendAttachmentInfo.blendEnable = VK_TRUE;
		colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_NO_OP;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachmentInfo;
		colorBlendStateInfo.blendConstants[0] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[1] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[2] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[3] = 0.0f;		// optional

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		if (descriptorSetLayout.has_value())
		{
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.value();
		}
		else
		{
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
		}
		pipelineLayoutInfo.pushConstantRangeCount = 0;			// optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr;		// optional

		if (vkCreatePipelineLayout(device.GetVKDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		// Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisamplingInfo;
		pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDynamicState = nullptr; // TODO: DOODLE - REINTEGRATE
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		// optional
		pipelineInfo.basePipelineIndex = -1;					// optional

		if (vkCreateGraphicsPipelines(device.GetVKDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		// Delete shader modules
		vkDestroyShaderModule(device.GetVKDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(device.GetVKDevice(), vertShaderModule, nullptr);
	}

	void Pipeline::CreateComputePipeline(const VkDescriptorSetLayout& descriptorSetLayout, const std::string& computeShaderFilePath)
	{
		// Shader Code
		std::vector<char> computeShaderCode = ReadFile(computeShaderFilePath);

		// Shader Modules
		VkShaderModule computeShaderModule = CreateShaderModule(computeShaderCode);

		// Compute Shader
		VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";

		// Push Constants
		VkPushConstantRange pushConstantRangeInfo = {};
		pushConstantRangeInfo.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRangeInfo.offset = 0;
		pushConstantRangeInfo.size = sizeof(PushConstants);

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRangeInfo;

		if (vkCreatePipelineLayout(device.GetVKDevice(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create compute pipeline layout!");
		}

		// Pipeline
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = computePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(device.GetVKDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create compute pipeline!");
		}

		// Delete shader modules
		vkDestroyShaderModule(device.GetVKDevice(), computeShaderModule, nullptr);
	}

	VkShaderModule Pipeline::CreateShaderModule(const std::vector<char>& code) const
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device.GetVKDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}

		return shaderModule;
	}

} // namespace VulkanCore
