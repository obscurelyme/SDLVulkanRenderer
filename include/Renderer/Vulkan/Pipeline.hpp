#ifndef _coffeemaker_renderer_vulkan_pipeline_hpp
#define _coffeemaker_renderer_vulkan_pipeline_hpp

#include <vulkan/vulkan.h>

#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  struct VertexInputDescription {
    std::vector<VkVertexInputBindingDescription> bindings{};
    std::vector<VkVertexInputAttributeDescription> attributes{};
    VkPipelineVertexInputStateCreateFlags flags = 0;
  };

  VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkShaderStageFlagBits stage,
                                                                VkShaderModule shaderModule);

  VkPipelineShaderStageCreateInfo CreateVertexShaderInfo(VkShaderModule shaderModule);

  VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo(VkShaderModule shaderModule);

  VkPipelineVertexInputStateCreateInfo CreateVertexInputInfo(VertexInputDescription& inputs);

  VkPipelineInputAssemblyStateCreateInfo CreateInputAssembly(
      VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  VkViewport CreateViewport();

  VkPipelineViewportStateCreateInfo CreateViewportState(VkViewport& viewport, VkRect2D& scissor);

  VkRect2D CreateScissor();

  /**
   * Possible params...
   *
   * VK_POLYGON_MODE_FILL = 0,
   * VK_POLYGON_MODE_LINE = 1,
   * VK_POLYGON_MODE_POINT = 2,
   * VK_POLYGON_MODE_FILL_RECTANGLE_NV = 1000153000,
   * VK_POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF
   */
  VkPipelineRasterizationStateCreateInfo CreateRasterizer(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL);

  VkPipelineColorBlendAttachmentState CreateColorBlendAttachState();

  VkPipelineColorBlendStateCreateInfo CreateColorBlendState(VkPipelineColorBlendAttachmentState& attachment);

  VkPipelineMultisampleStateCreateInfo MultiSampling();

  VkPipelineDepthStencilStateCreateInfo CreateDepthStencilCreateInfo(
      bool bDepthTest = true, bool bDepthWrite = true, VkCompareOp compareOp = VK_COMPARE_OP_LESS_OR_EQUAL);

  VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo(uint32_t pushConstantRangeCount = 0,
                                                      VkPushConstantRange* pushConstant = nullptr);

  struct PipelineCreateInfo {
    VkShaderModule vertexShader{VK_NULL_HANDLE};
    VkShaderModule fragmentShader{VK_NULL_HANDLE};
    VertexInputDescription vertexInputs;
    uint32_t pushConstantRangeCount = 0;
    VkPushConstantRange pushConstants{};
  };

  class Pipeline {
    public:
    void CreatePipeline(PipelineCreateInfo info);
    Pipeline();
    ~Pipeline();

    Pipeline(const Pipeline& p) = delete;
    void operator=(const Pipeline& p) = delete;

    VkPipeline pPipeline{VK_NULL_HANDLE};
    VkPipelineLayoutCreateInfo layoutInfo{};
    VkPipelineLayout layout{};
    PipelineCreateInfo info;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkViewport viewport{};
    VkRect2D scissor{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
  };
}  // namespace CoffeeMaker::Renderer::Vulkan

#endif