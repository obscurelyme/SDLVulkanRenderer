#include "Renderer/Vulkan/Pipeline.hpp"

#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/RenderPass.hpp"

VkPipelineShaderStageCreateInfo CoffeeMaker::Renderer::Vulkan::CreatePipelineShaderStageInfo(
    VkShaderStageFlagBits stage, VkShaderModule shaderModule) {
  VkPipelineShaderStageCreateInfo info{};

  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.pNext = nullptr;
  // NOTE: shader stage
  info.stage = stage;
  // NOTE: module containing the code for this shader stage
  info.module = shaderModule;
  // NOTE: entry point function of the shader
  info.pName = "main";

  return info;
}

VkPipelineShaderStageCreateInfo CoffeeMaker::Renderer::Vulkan::CreateVertexShaderInfo(VkShaderModule shaderModule) {
  return CoffeeMaker::Renderer::Vulkan::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, shaderModule);
}

VkPipelineShaderStageCreateInfo CoffeeMaker::Renderer::Vulkan::CreateFragmentShaderInfo(VkShaderModule shaderModule) {
  return CoffeeMaker::Renderer::Vulkan::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModule);
}

VkPipelineVertexInputStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateVertexInputInfo(
    CoffeeMaker::Renderer::Vulkan::VertexInputDescription& inputs) {
  VkPipelineVertexInputStateCreateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  info.pNext = nullptr;

  info.vertexBindingDescriptionCount = inputs.bindings.size();
  info.pVertexBindingDescriptions = inputs.bindings.size() > 0 ? inputs.bindings.data() : nullptr;
  info.vertexAttributeDescriptionCount = inputs.attributes.size();
  info.pVertexAttributeDescriptions = inputs.attributes.size() > 0 ? inputs.attributes.data() : nullptr;

  return info;
}

VkPipelineInputAssemblyStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateInputAssembly(
    VkPrimitiveTopology topology) {
  VkPipelineInputAssemblyStateCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  info.pNext = nullptr;

  info.topology = topology;
  // we are not going to use primitive restart on the entire tutorial so leave it on false
  info.primitiveRestartEnable = VK_FALSE;

  return info;
}

VkViewport CoffeeMaker::Renderer::Vulkan::CreateViewport() {
  VkViewport vp{};

  vp.x = 0.0f;
  vp.y = 0.0f;
  vp.width = 1280.0f;  // TODO: get from swapchain
  vp.height = 720.0f;  // TODO: get from swapchain
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;

  return vp;
}

VkPipelineViewportStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateViewportState(VkViewport& viewport,
                                                                                     VkRect2D& scissor) {
  VkPipelineViewportStateCreateInfo viewportState{};

  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.pNext = nullptr;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  return viewportState;
}

VkRect2D CoffeeMaker::Renderer::Vulkan::CreateScissor() {
  VkRect2D rect{};
  VkExtent2D extent{};
  extent.width = static_cast<uint32_t>(1280);
  extent.height = static_cast<uint32_t>(720);

  rect.offset.x = static_cast<int32_t>(0);
  rect.offset.y = static_cast<int32_t>(0);
  rect.extent = extent;

  return rect;
}

/**
 * Possible params...
 *
 * VK_POLYGON_MODE_FILL = 0,
 * VK_POLYGON_MODE_LINE = 1,
 * VK_POLYGON_MODE_POINT = 2,
 * VK_POLYGON_MODE_FILL_RECTANGLE_NV = 1000153000,
 * VK_POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF
 */
VkPipelineRasterizationStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateRasterizer(VkPolygonMode polygonMode) {
  VkPipelineRasterizationStateCreateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.depthClampEnable = VK_FALSE;
  // NOTE: discards all primitives before the rasterization stage if enabled which we don't want
  info.rasterizerDiscardEnable = VK_FALSE;
  info.polygonMode = polygonMode;
  info.lineWidth = 1.0f;
  // NOTE: no backface culling
  info.cullMode = VK_CULL_MODE_NONE;
  info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  // NOTE: no depth bias
  info.depthBiasEnable = VK_FALSE;
  info.depthBiasConstantFactor = 0.0f;  // Optional
  info.depthBiasClamp = 0.0f;           // Optional
  info.depthBiasSlopeFactor = 0.0f;     // Optional

  return info;
}

VkPipelineColorBlendAttachmentState CoffeeMaker::Renderer::Vulkan::CreateColorBlendAttachState() {
  VkPipelineColorBlendAttachmentState info = {};

  info.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  info.blendEnable = VK_FALSE;

  return info;
}

VkPipelineColorBlendStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateColorBlendState(
    VkPipelineColorBlendAttachmentState& attachment) {
  VkPipelineColorBlendStateCreateInfo info{};

  info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.logicOpEnable = VK_FALSE;
  info.logicOp = VK_LOGIC_OP_COPY;  // Optional
  info.attachmentCount = 1;
  info.pAttachments = &attachment;
  info.blendConstants[0] = 0.0f;  // Optional
  info.blendConstants[1] = 0.0f;  // Optional
  info.blendConstants[2] = 0.0f;  // Optional
  info.blendConstants[3] = 0.0f;  // Optional

  return info;
}

VkPipelineMultisampleStateCreateInfo CoffeeMaker::Renderer::Vulkan::MultiSampling() {
  VkPipelineMultisampleStateCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  info.pNext = nullptr;

  info.sampleShadingEnable = VK_FALSE;
  // multisampling defaulted to no multisampling (1 sample per pixel)
  info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  info.minSampleShading = 1.0f;
  info.pSampleMask = nullptr;
  info.alphaToCoverageEnable = VK_FALSE;
  info.alphaToOneEnable = VK_FALSE;

  return info;
}

VkPipelineDepthStencilStateCreateInfo CoffeeMaker::Renderer::Vulkan::CreateDepthStencilCreateInfo(
    bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp) {
  VkPipelineDepthStencilStateCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info.pNext = nullptr;

  info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
  info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
  info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
  info.depthBoundsTestEnable = VK_FALSE;
  info.minDepthBounds = 0.0f;  // Optional
  info.maxDepthBounds = 1.0f;  // Optional
  info.stencilTestEnable = VK_FALSE;

  return info;
}

VkPipelineLayoutCreateInfo CoffeeMaker::Renderer::Vulkan::CreatePipelineLayoutInfo(uint32_t pushConstantRangeCount,
                                                                                   VkPushConstantRange* pushConstant) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.pNext = nullptr;

  // empty defaults
  info.flags = 0;
  info.setLayoutCount = 0;
  info.pSetLayouts = nullptr;
  // push constants
  info.pushConstantRangeCount = pushConstantRangeCount;
  info.pPushConstantRanges = pushConstant;

  return info;
}

void CoffeeMaker::Renderer::Vulkan::Pipeline::CreatePipeline(CoffeeMaker::Renderer::Vulkan::PipelineCreateInfo info) {
  using RenderPass = CoffeeMaker::Renderer::Vulkan::RenderPass;
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VkGraphicsPipelineCreateInfo pipelineInfo = {};

  shaderStages.push_back(CreateVertexShaderInfo(info.vertexShader));
  shaderStages.push_back(CreateFragmentShaderInfo(info.fragmentShader));
  vertexInputInfo = CreateVertexInputInfo(info.vertexInputs);
  inputAssembly = CreateInputAssembly();
  viewport = CreateViewport();
  scissor = CreateScissor();
  viewportState = CreateViewportState(viewport, scissor);
  rasterizer = CreateRasterizer();
  colorBlendAttachment = CreateColorBlendAttachState();
  colorBlending = CreateColorBlendState(colorBlendAttachment);
  multisampling = MultiSampling();
  depthStencil = CreateDepthStencilCreateInfo();
  layoutInfo = CreatePipelineLayoutInfo(info.pushConstantRangeCount,
                                        info.pushConstantRangeCount == 0 ? nullptr : &info.pushConstants);

  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = dynamicStates.size();
  dynamicState.pDynamicStates = dynamicStates.data();

  VkResult result = vkCreatePipelineLayout(LogicalDevice::GetLogicalDevice(), &layoutInfo, nullptr, &layout);
  if (result != VK_SUCCESS) {
    abort();
  }

  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = shaderStages.size();
  pipelineInfo.pStages = shaderStages.data();
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = RenderPass::GetRenderPass();
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex = -1;               // Optional

  result = vkCreateGraphicsPipelines(LogicalDevice::GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                     &pPipeline);
  if (result != VK_SUCCESS) {
    exit(12);
  }
}

CoffeeMaker::Renderer::Vulkan::Pipeline::Pipeline() = default;

CoffeeMaker::Renderer::Vulkan::Pipeline::~Pipeline() {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  VkDevice ld = LogicalDevice::GetLogicalDevice();

  vkDeviceWaitIdle(ld);
  vkDestroyPipeline(ld, pPipeline, nullptr);
  vkDestroyPipelineLayout(ld, layout, nullptr);
}
