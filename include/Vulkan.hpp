#ifndef _coffeemaker_vulkan_hpp
#define _coffeemaker_vulkan_hpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Triangle.hpp"
#include "VulkanCommands.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanSync.hpp"

class Vulkan {
  public:
  friend class VulkanShaderManager;

  explicit Vulkan(const std::string &applicationName, SDL_Window *window);
  ~Vulkan();

  VkInstance InstanceHandle() const;
  VkDevice LogicalDevice() const { return logicalDevice.Handle; }

  void Draw2();

  void Draw();
  void RecreateSwapChain();
  void FramebufferResize();

  /**
   * @brief VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like
   * the creation of a resource
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior
   * that is not necessarily an error, but very likely a bug in your application
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that
   * is invalid and may cause crashes
   *
   * @param messageSeverity
   * @param messageType
   * @param pCallbackData
   * @param pUserData
   * @return VKAPI_ATTR
   */
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::cerr << "[KHS_Validation_Layer]: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
  }

  private:
  void CleanupSwapChain();
  void InitVulkan();
  bool CheckValidationLayerSupport();
  void GetRequiredExtensions();
  void SetupDebugMessenger();
  void SetupValidationLayers();
  VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator);
  void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks *pAllocator);
  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  void PickPhysicalDevice();
  bool IsDeviceSuitable(VulkanPhysicalDevice &device);
  void CreateLogicalDevice();
  /**
   * @brief After choosing a physical device, query what extensions are required
   * by Vulkan in order to create a logical device later.
   */
  void AddRequiredDeviceExtensionSupport(VkPhysicalDevice device);
  void CreateSwapChain();
  void CreateGraphicsPipeline();
  template <size_t N>
  void SetupFixedFunctionsPipeline(std::array<VkPipelineShaderStageCreateInfo, N> shaderStages) {
    VkExtent2D swapChainExtent = swapChain.GetExtent();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizer.depthBiasClamp = 0.0f;           // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;           // Optional
    multisampling.pSampleMask = nullptr;             // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable = VK_FALSE;       // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;  // Optional
    colorBlending.blendConstants[1] = 0.0f;  // Optional
    colorBlending.blendConstants[2] = 0.0f;  // Optional
    colorBlending.blendConstants[3] = 0.0f;  // Optional

    std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;             // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;          // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;     // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional

    VkResult result = vkCreatePipelineLayout(logicalDevice.Handle, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
      ShowError("Vulkan Pipeline Layout", fmt::format("Unable to create a Vulkan pipeline layout.\nVulkan "
                                                      "Error Code : [{}]",
                                                      result));
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;  // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;  // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass.Handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    result =
        vkCreateGraphicsPipelines(logicalDevice.Handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != VK_SUCCESS) {
      ShowError("Vulkan Graphics Pipeline", fmt::format("Unable to create Vulkan Graphics Pipeline.\nVulkan "
                                                        "Error Code: [{}]",
                                                        result));
    }
  }
  void CreateRenderPass();
  void CreateFramebuffer();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateCommands();
  void CreateSemaphores();
  void CreateMemoryAllocator();

  void CreateSurface();

  void ShowError(const std::string &title, const std::string &message);

  bool enableValidationLayers;
  bool vulkanInstanceInitialized;
  SDL_Window *windowHandle;
  VkInstance vulkanInstance;
  VkApplicationInfo vulkanAppInfo;
  VkInstanceCreateInfo vulkanInstanceCreateInfo;

  uint32_t REQUIRED_VULKAN_EXTENSIONS_COUNT;
  std::vector<const char *> REQUIRED_VULKAN_EXTENSIONS{};
  uint32_t VULKAN_LAYERS_COUNT;
  std::vector<const char *> VULKAN_LAYERS{};
  std::vector<const char *> REQUIRED_VULKAN_DEVICE_EXTENSIONS{};
  std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR vulkanSurface;

  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  static int MAX_FRAMES_IN_FLIGHT;
  bool framebufferResized;

  VmaAllocator allocator;
  VulkanPhysicalDevice physicalDevice{nullptr};
  VulkanLogicalDevice logicalDevice;
  VulkanSwapchain swapChain;
  VulkanCommands commands;
  VulkanRenderPass renderPass;
  VulkanFramebuffer framebuffer;
  VulkanSync syncUtils;
  int framecount;

  Triangle *triangle;
};

#endif
