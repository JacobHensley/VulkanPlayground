#include "pch.h"
#include "VulkanPipeline.h"
#include "VulkanPlayground/Core/Application.h"
#include "VulkanPlayground/Core/VulkanTools.h"

namespace VKPlayground {

	static const VkDynamicState s_DynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VulkanPipeline::VulkanPipeline(Ref<Shader> shader, VkRenderPass renderPass)
		: m_Shader(shader), m_RenderPass(renderPass)
	{
		Init();
		LOG_INFO("Initialized Vulkan pipeline");
	}

	VulkanPipeline::~VulkanPipeline()
	{
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();

		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	void VulkanPipeline::Init()
	{
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes(4);

		// Vertex 0: Position
		vertexInputAttributes[0].binding = 0;
		vertexInputAttributes[0].location = 0;
		vertexInputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributes[0].offset = 0;

		// Vertex 0: Normal
		vertexInputAttributes[1].binding = 0;
		vertexInputAttributes[1].location = 1;
		vertexInputAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[1].offset = 12;

		// Vertex 0: Tangent
		vertexInputAttributes[2].binding = 0;
		vertexInputAttributes[2].location = 2;
		vertexInputAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[2].offset = 24;

		// Vertex 0: TextureCoords
		vertexInputAttributes[3].binding = 0;
		vertexInputAttributes[3].location = 2;
		vertexInputAttributes[3].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[3].offset = 36;

		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2); // Size of entire vertex
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Create vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

		// Create input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		VkExtent2D swapChainExtent = swapChain->GetExtent();

		// Create viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = (float)swapChainExtent.height;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = -(float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// Create scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		// Create viewport state
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Create rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f;          // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

		// Disable multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;          // Optional
		multisampling.pSampleMask = nullptr;            // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE;      // Optional

		// Color blending attachment
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

		// Color blend state (Disable color blending)
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;   // Optional
		colorBlending.blendConstants[1] = 0.0f;   // Optional
		colorBlending.blendConstants[2] = 0.0f;   // Optional
		colorBlending.blendConstants[3] = 0.0f;   // Optional

		// Set dynamic states
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = s_DynamicStates;

		//Set push constants
		VkPushConstantRange pushConstantRange;
		pushConstantRange.offset = 0;
		pushConstantRange.size = 128; // TODO: Get this info from device
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// Set pipeline layout
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = m_Shader->GetDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;    // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		const std::vector<VkPipelineShaderStageCreateInfo>& shaderCreateInfo = m_Shader->GetShaderCreateInfo();
	//	VkRenderPass renderPass = swapChain->GetRenderPass();
		VkRenderPass renderPass = m_RenderPass;

		// Create pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderCreateInfo.size();
		pipelineInfo.pStages = shaderCreateInfo.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;         // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1;              // Optional

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
	}

}