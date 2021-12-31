#ifndef _coffeemaker_vulkan_graphics_pipeline_hpp
#define _coffeemaker_vulkan_graphics_pipeline_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "SimpleMessageBox.hpp"
#include "VkUtils.hpp"
#include "VulkanMesh.hpp"

/**
 * @brief Builder class for VkPipeline structs
 */
class PipelineBuilder {
  public:
  std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
  VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
  VkViewport _viewport;
  VkRect2D _scissor;
  VkPipelineRasterizationStateCreateInfo _rasterizer;
  VkPipelineColorBlendAttachmentState _colorBlendAttachment;
  VkPipelineMultisampleStateCreateInfo _multisampling;
  VkPipelineLayout _pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo _depthStencil;

  /**
   * @brief Inserts a shader module into the render pipeline
   */
  auto ShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule) -> PipelineBuilder& {
    VkPipelineShaderStageCreateInfo info{};

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;
    // NOTE: shader stage
    info.stage = stage;
    // NOTE: module containing the code for this shader stage
    info.module = shaderModule;
    // NOTE: entry point function of the shader
    info.pName = "main";

    _shaderStages.push_back(info);

    return *this;
  }

  /**
   * @brief Sets information about Vertex buffers and formats for the shaders
   */
  auto VertexInputInfo() -> PipelineBuilder& {
    VkPipelineVertexInputStateCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    // NOTE: no vertex bindings or attributes
    info.vertexBindingDescriptionCount = 0;
    info.pVertexBindingDescriptions = nullptr;
    info.vertexAttributeDescriptionCount = 0;
    info.pVertexAttributeDescriptions = nullptr;

    _vertexInputInfo = info;

    return *this;
  }

  /**
   * @brief Sets information about Vertex buffers and formats for the shaders
   */
  auto VertexInputInfo(VertexInputDescription inputDescription) -> PipelineBuilder& {
    VkPipelineVertexInputStateCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    // NOTE: no vertex bindings or attributes
    info.vertexBindingDescriptionCount = inputDescription.bindings.size();
    info.pVertexBindingDescriptions = inputDescription.bindings.data();
    info.vertexAttributeDescriptionCount = inputDescription.attributes.size();
    info.pVertexAttributeDescriptions = inputDescription.attributes.data();

    _vertexInputInfo = info;

    return *this;
  }

  /**
   * @brief Specifies what kind of topology will be drawn.
   */
  auto InputAssembly(VkPrimitiveTopology topology) -> PipelineBuilder& {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    // we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_FALSE;

    _inputAssembly = info;

    return *this;
  }

  /**
   * @brief Configure the pipeline viewport from the Swapchain extents.
   *
   * @param x Typically 0.0f
   * @param y Typically 0.0f
   * @param width Desired width from the swapchain extent
   * @param height Desired height from the swapchain extent
   * @param minDepth Typically 0.0f
   * @param maxDepth Typically 1.0f
   */
  auto Viewport(float x, float y, float width, float height, float minDepth, float maxDepth) -> PipelineBuilder& {
    VkViewport v{.x = x, .y = y, .width = width, .height = height, .minDepth = minDepth, .maxDepth = maxDepth};
    _viewport = v;
    return *this;
  }

  /**
   * @brief Configure the Scissor object from the Swapchain extents.
   */
  auto Scissor(int32_t xOffset, int32_t yOffset, VkExtent2D swapChainExtent) -> PipelineBuilder& {
    _scissor.offset.x = xOffset;
    _scissor.offset.y = yOffset;
    _scissor.extent = swapChainExtent;

    return *this;
  }

  /**
   * @brief Configuration for the fixed-function rasterization.
   */
  auto Rasterizer(VkPolygonMode polygonMode) -> PipelineBuilder& {
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

    _rasterizer = info;

    return *this;
  }

  /**
   * @brief Controls how this pipeline blends into a given attachment.
   */
  auto ColorBlendAttachState() -> PipelineBuilder& {
    VkPipelineColorBlendAttachmentState info = {};

    info.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    info.blendEnable = VK_FALSE;
    _colorBlendAttachment = info;

    return *this;
  }

  /**
   * @brief Configure MSAA for the pipeline. Note that the renderpass needs to support
   * MSAA if this is to be enabled.
   */
  auto MultiSampling() -> PipelineBuilder& {
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
    _multisampling = info;

    return *this;
  }

  auto PipelineLayout(VkDevice device) -> PipelineBuilder& {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    // empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &_pipelineLayout);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Pipeline Layout",
                                  fmt::format("Unable to create a Vulkan pipeline layout.\nVulkan "
                                              "Error Code : [{}]",
                                              result));
    }

    return *this;
  }

  auto PipelineLayout(VkDevice device, VkPushConstantRange pushConstant) -> PipelineBuilder& {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    // empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    // push constants
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &pushConstant;

    VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &_pipelineLayout);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Pipeline Layout",
                                  fmt::format("Unable to create a Vulkan pipeline layout.\nVulkan "
                                              "Error Code : [{}]",
                                              result));
    }

    return *this;
  }

  PipelineBuilder& DepthStencil() {
    _depthStencil = CreateDepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
    return *this;
  }

  void Reset() {
    _shaderStages.clear();
    _vertexInputInfo = {};
    _inputAssembly = {};
    _viewport = {};
    _scissor = {};
    _rasterizer = {};
    _colorBlendAttachment = {};
    _multisampling = {};
    _pipelineLayout = {};
    _depthStencil = {};
  }

  auto Build(VkDevice device, VkRenderPass renderPass) -> VkPipeline {
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &_viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &_scissor;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &_colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;  // Optional
    colorBlending.blendConstants[1] = 0.0f;  // Optional
    colorBlending.blendConstants[2] = 0.0f;  // Optional
    colorBlending.blendConstants[3] = 0.0f;  // Optional

    // NOTE: Use all of the info structs to build the actual pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = _shaderStages.size();
    pipelineInfo.pStages = _shaderStages.data();
    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &_inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &_rasterizer;
    pipelineInfo.pMultisampleState = &_multisampling;
    pipelineInfo.pDepthStencilState = &_depthStencil;  // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;  // Optional
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    VkPipeline newPipeline;
    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Graphics Pipeline",
                                  fmt::format("Unable to create Vulkan Graphics Pipeline.\nVulkan "
                                              "Error Code: [{}]",
                                              result));
    }

    return newPipeline;
  }
};

#endif