#include "Pipeline.h"

#include <fstream>
#include <iostream>

namespace VulkanCore {

	const std::vector<VkDynamicState> Pipeline::dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	Pipeline::Pipeline(GPUDevice& device, const VkRenderPass& renderPass)
		: device(device)
	{
		CreateGraphicsPipeline(renderPass);
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(device.GetVKDevice(), graphicsPipeline, nullptr);

		// TODO: move
		vkDestroyPipelineLayout(device.GetVKDevice(), pipelineLayout, nullptr);
	}

	void Pipeline::Bind(VkCommandBuffer commandBuffer)
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

	void Pipeline::CreateGraphicsPipeline(const VkRenderPass& renderPass)
	{
		// TODO: test
#if defined(PLATFORM_WINDOWS) || (defined(PLATFORM_LINUX) && defined(NDEBUG))
		static const std::string vertShaderFilePath = "shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "shaders/triangle.frag.spv";
#elif defined(PLATFORM_LINUX) && defined(DEBUG)
		static const std::string vertShaderFilePath = "ParticleSystem/shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "ParticleSystem/shaders/triangle.frag.spv";
#endif

		std::vector<char> vertShaderCode = ReadFile(vertShaderFilePath);
		std::vector<char> fragShaderCode = ReadFile(fragShaderFilePath);

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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;	// Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport + Scissors as Dynamic States
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;	// optional
		rasterizationInfo.depthBiasClamp = 0.0f;			// optional
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;		// optional

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingInfo.minSampleShading = 1.0f;				// optional
		multisamplingInfo.pSampleMask = nullptr;				// optional
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE;		// optional
		multisamplingInfo.alphaToOneEnable = VK_FALSE;			// optional

		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo = {};
		colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentInfo.blendEnable = VK_FALSE;						// TODO: enable blending
		colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// optional
		colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;				// optional
		colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
		colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;				// optional

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;		// optional
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachmentInfo;
		colorBlendStateInfo.blendConstants[0] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[1] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[2] = 0.0f;		// optional
		colorBlendStateInfo.blendConstants[3] = 0.0f;		// optional

		// TODO: move
		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;				// optional
		pipelineLayoutInfo.pSetLayouts = nullptr;			// optional
		pipelineLayoutInfo.pushConstantRangeCount = 0;		// optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr;	// optional

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
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisamplingInfo;
		pipelineInfo.pDepthStencilState = nullptr;				// optional
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		// optional
		pipelineInfo.basePipelineIndex = -1;					// optional

		if (vkCreateGraphicsPipelines(device.GetVKDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device.GetVKDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(device.GetVKDevice(), vertShaderModule, nullptr);
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
