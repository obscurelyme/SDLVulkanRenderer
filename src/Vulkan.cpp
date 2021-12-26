#include "Vulkan.hpp"
#include "VulkanShaderManager.hpp"
#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <set>
#include <vector>

int Vulkan::MAX_FRAMES_IN_FLIGHT = 2;

Vulkan::Vulkan(const std::string &applicationName, SDL_Window *window)
    : enableValidationLayers(true), vulkanInstanceInitialized(false),
      windowHandle(window), vulkanAppInfo({}), vulkanInstanceCreateInfo({}), framebufferResized(false) {
  vulkanAppInfo.pApplicationName = applicationName.c_str();
  vulkanAppInfo.pEngineName = applicationName.c_str();

  SetupValidationLayers();
  InitVulkan();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffer();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSemaphores();
}

Vulkan::~Vulkan() {
  vkDeviceWaitIdle(logicalDevice.Handle);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(nullptr);
  }

  CleanupSwapChain();

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(logicalDevice.Handle, renderFinishedSemaphores[i],
                       nullptr);
    vkDestroySemaphore(logicalDevice.Handle, imageAvailableSemaphores[i],
                       nullptr);
    vkDestroyFence(logicalDevice.Handle, inFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(logicalDevice.Handle, commandPool, nullptr);
  logicalDevice.DestroyHandle();
  if (vulkanInstanceInitialized) {
    vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
  }
}

void Vulkan::CleanupSwapChain() {
  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(logicalDevice.Handle, framebuffer, nullptr);
  }
  vkFreeCommandBuffers(logicalDevice.Handle, commandPool,
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  vkDestroyPipeline(logicalDevice.Handle, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(logicalDevice.Handle, pipelineLayout, nullptr);
  vkDestroyRenderPass(logicalDevice.Handle, renderPass, nullptr);
  swapChain.DestroyHandle();
}

void Vulkan::FramebufferResize() { framebufferResized = true; }

void Vulkan::RecreateSwapChain() {
  vkDeviceWaitIdle(logicalDevice.Handle);

  CleanupSwapChain();
  vkDestroyCommandPool(logicalDevice.Handle, commandPool, nullptr);

  CreateSwapChain();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffer();
  CreateCommandPool();
  CreateCommandBuffers();
}

void Vulkan::Draw() {
  vkWaitForFences(logicalDevice.Handle, 1, &inFlightFences[currentFrame],
                  VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult nxtImageResult = vkAcquireNextImageKHR(
      logicalDevice.Handle, swapChain.Handle, UINT64_MAX,
      imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (nxtImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return;
  } else if (nxtImageResult != VK_SUCCESS &&
             nxtImageResult != VK_SUBOPTIMAL_KHR) {
    ShowError(
        "Vulkan Draw",
        fmt::format(
            "Vulkan Draw error occured, could not acquire next image.\n[{}]",
            nxtImageResult));
  }

  if (nxtImageResult != VK_SUCCESS) {
    ShowError(
        "Vulkan Draw",
        fmt::format(
            "Vulkan Draw error occured, could not acquire next image.\n[{}]",
            nxtImageResult));
  }

  if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(logicalDevice.Handle, 1, &imagesInFlight[imageIndex],
                    VK_TRUE, UINT64_MAX);
  }
  imagesInFlight[imageIndex] = inFlightFences[currentFrame];

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(logicalDevice.Handle, 1, &inFlightFences[currentFrame]);
  VkResult result = vkQueueSubmit(logicalDevice.GraphicsQueue, 1, &submitInfo,
                                  inFlightFences[currentFrame]);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      framebufferResized) {
    std::cerr << "Recreating swap chain..." << std::endl;
    framebufferResized = false;
    RecreateSwapChain();
  } else if (result != VK_SUCCESS) {
    ShowError(
        "Vulkan Queue Submit",
        fmt::format("Failed to submit draw command to buffer: [{}]", result));
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain.Handle};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr; // Optional

  vkQueuePresentKHR(logicalDevice.PresentQueue, &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::InitVulkan() {
  // Check for Layer Support
  CheckValidationLayerSupport();

  // Get all supported vulkan instance extensions
  uint32_t extensionSupportCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionSupportCount,
                                         nullptr);
  std::vector<VkExtensionProperties> supportedExtensions(extensionSupportCount);
  std::vector<const char *> supportedExtensionNames{};
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionSupportCount,
                                         supportedExtensions.data());

  std::cout << "Supported Extensions" << std::endl;
  for (auto &supportedExtension : supportedExtensions) {
    std::cout << supportedExtension.extensionName << std::endl;
    supportedExtensionNames.push_back(supportedExtension.extensionName);
  }

  // Application Info
  vulkanAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  vulkanAppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  vulkanAppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  vulkanAppInfo.apiVersion = VK_API_VERSION_1_0;

  // Required Extensions
  GetRequiredExtensions();

  // Confirm required extensions are supported
  for (auto requiredExtension : REQUIRED_VULKAN_EXTENSIONS) {
    bool found = false;
    for (size_t i = 0; i < supportedExtensionNames.size(); ++i) {
      if (strcmp(requiredExtension, supportedExtensionNames[i]) == 0) {
        found = true;
      }
    }
    if (!found) {
      ShowError("Vulkan Init Error",
                fmt::format("Specified extension is NOT supported by this "
                            "machine\n Vulkan Extension Requested: [{}]",
                            requiredExtension));
    }
  }

  // Instance Creation Info
  vulkanInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vulkanInstanceCreateInfo.pApplicationInfo = &vulkanAppInfo;
  vulkanInstanceCreateInfo.enabledLayerCount = 0;
  vulkanInstanceCreateInfo.ppEnabledLayerNames = nullptr;
  vulkanInstanceCreateInfo.enabledExtensionCount =
      REQUIRED_VULKAN_EXTENSIONS_COUNT;
  vulkanInstanceCreateInfo.ppEnabledExtensionNames =
      REQUIRED_VULKAN_EXTENSIONS.data();
  vulkanInstanceCreateInfo.pNext = nullptr;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers) {
    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    vulkanInstanceCreateInfo.enabledLayerCount = VULKAN_LAYERS.size();
    vulkanInstanceCreateInfo.ppEnabledLayerNames = VULKAN_LAYERS.data();
    vulkanInstanceCreateInfo.pNext =
        (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  }

  VkResult result =
      vkCreateInstance(&vulkanInstanceCreateInfo, nullptr, &vulkanInstance);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Init Error",
              fmt::format("Error resulted in creating vulkan instance\n Vulkan "
                          "Error Code: [{}]",
                          result));
  }
  vulkanInstanceInitialized = true;
}

VkInstance Vulkan::InstanceHandle() const { return vulkanInstance; }

void Vulkan::ShowError(const std::string &title, const std::string &message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(),
                           windowHandle);
  exit(1);
}

bool Vulkan::CheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::cout << "Supported Layers" << std::endl;
  for (auto &availableLayer : availableLayers) {
    fmt::print("Name: {}\nDescription: {}\n", availableLayer.layerName,
               availableLayer.description);
  }

  for (const char *layerName : VULKAN_LAYERS) {
    bool layerFound = false;
    for (size_t i = 0; i < availableLayers.size(); ++i) {
      if (strcmp(layerName, availableLayers[i].layerName) == 0) {
        layerFound = true;
      }
    }
    if (!layerFound) {
      fmt::print("Specified Layer is NOT supported by this "
                 "machine\n Vulkan Layer Requested: [{}]\n",
                 layerName);
      ShowError("Vulkan Init Error",
                fmt::format("Specified Layer is NOT supported by this "
                            "machine\n Vulkan Layer Requested: [{}]",
                            layerName));
    }
  }

  return true;
}

void Vulkan::GetRequiredExtensions() {
  // SDL Required Extensions
  SDL_Vulkan_GetInstanceExtensions(windowHandle,
                                   &REQUIRED_VULKAN_EXTENSIONS_COUNT, nullptr);
  REQUIRED_VULKAN_EXTENSIONS.resize(REQUIRED_VULKAN_EXTENSIONS_COUNT);
  SDL_Vulkan_GetInstanceExtensions(windowHandle,
                                   &REQUIRED_VULKAN_EXTENSIONS_COUNT,
                                   REQUIRED_VULKAN_EXTENSIONS.data());

  // Debug utils
  if (enableValidationLayers) {
    REQUIRED_VULKAN_EXTENSIONS.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    REQUIRED_VULKAN_EXTENSIONS_COUNT++;
  }
  REQUIRED_VULKAN_EXTENSIONS.push_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  REQUIRED_VULKAN_EXTENSIONS_COUNT++;
}

void Vulkan::SetupValidationLayers() {
  if (!enableValidationLayers) {
    return;
  }

  VULKAN_LAYERS_COUNT = 1;
  VULKAN_LAYERS.push_back("VK_LAYER_KHRONOS_validation");
  // VULKAN_LAYERS.push_back("VK_LAYER_KHRONOS_validation");
}

void Vulkan::PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr; // Optional user data
}

void Vulkan::SetupDebugMessenger() {
  if (!enableValidationLayers) {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  PopulateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

VkResult Vulkan::CreateDebugUtilsMessengerEXT(
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      vulkanInstance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(vulkanInstance, pCreateInfo, pAllocator, &debugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void Vulkan::DestroyDebugUtilsMessengerEXT(
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      vulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(vulkanInstance, debugMessenger, pAllocator);
  }
}

void Vulkan::PickPhysicalDevice() {
  std::vector<VulkanPhysicalDevice> devices = VulkanPhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);


  for (VulkanPhysicalDevice &device : devices) {
    device.SetSurface(vulkanSurface);
    device.FindQueueFamilies();
    if (IsDeviceSuitable(device)) {
      physicalDevice = device;
      AddRequiredDeviceExtensionSupport(physicalDevice.Handle);
      break;
    }
  }

  if (physicalDevice.Handle == VK_NULL_HANDLE) {
    ShowError("Vulkan Physical Device", "Unable to find suitable GPU.");
  }
}

bool Vulkan::IsDeviceSuitable(VulkanPhysicalDevice& device) {
  bool swapChainAdequate = false;
  bool extensionsSupported = device.AreExtensionsSupported(deviceExtensions);
  if (extensionsSupported) {
    device.QuerySwapChainSupport();
    swapChainAdequate = !device.SwapChainSupport.formats.empty() &&
                        !device.SwapChainSupport.presentModes.empty();
  }

  fmt::print("Checking device: {}\n", device.ToString());

  bool result =
      device.QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device.IsDiscreteGPU();

  if (result) {
    fmt::print("Using device: {}\n", device.Name());
  }

  return result;
}

void Vulkan::CreateLogicalDevice() {
  logicalDevice.SetPhysicalDevice(physicalDevice);
  logicalDevice.SetExentions(deviceExtensions);
  logicalDevice.SetLayers(VULKAN_LAYERS);
  logicalDevice.EnableValidation(enableValidationLayers);
  logicalDevice.SetUp();
  logicalDevice.Create();
}

void Vulkan::CreateSurface() {
  if (!SDL_Vulkan_CreateSurface(windowHandle, vulkanInstance, &vulkanSurface)) {
    ShowError("SDL Vulkan Surface",
              fmt::format(
                  "SDL2 was unable to create Vulkan Surface.\nSDL Error: [{}]",
                  SDL_GetError()));
  }
}

void Vulkan::AddRequiredDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());
  for (auto availExt : availableExtensions) {
    if (availExt.extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) {
      /**
       * If a physical device allows for this extension, then it MUST be enabled.
       * @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_portability_subset.html
       */
      deviceExtensions.push_back(availExt.extensionName);
    }
  }
}

void Vulkan::CreateSwapChain() {
  swapChain.SetPhysicalDevice(&physicalDevice);
  swapChain.SetLogicalDevice(&logicalDevice);
  swapChain.ChooseSwapSurfaceFormat();
  swapChain.ChoosePresentationMode();
  swapChain.ChooseSwapExtent();
  swapChain.Create();
  swapChain.CreateImageViews();
}

void Vulkan::CreateGraphicsPipeline() {
  auto fragShaderCode = VulkanShaderManager::ReadShaderFile("frag.spv");
  auto vertShaderCode = VulkanShaderManager::ReadShaderFile("vert.spv");

  VkShaderModule fragShaderModule = VulkanShaderManager::CreateShaderModule(
      logicalDevice.Handle, fragShaderCode);
  VkShaderModule vertShaderModule = VulkanShaderManager::CreateShaderModule(
      logicalDevice.Handle, vertShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  SetupFixedFunctionsPipeline(shaderStages);

  VulkanShaderManager::CleanupShaderModule(logicalDevice.Handle,
                                           fragShaderModule);
  VulkanShaderManager::CleanupShaderModule(logicalDevice.Handle,
                                           vertShaderModule);
}

void Vulkan::SetupFixedFunctionsPipeline(
    VkPipelineShaderStageCreateInfo shaderStages[]) {
  VkExtent2D swapChainExtent = swapChain.GetExtent();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
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
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = nullptr;            // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_LINE_WIDTH};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;            // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  VkResult result = vkCreatePipelineLayout(
      logicalDevice.Handle, &pipelineLayoutInfo, nullptr, &pipelineLayout);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Pipeline Layout",
              fmt::format("Unable to create a Vulkan pipeline layout.\nVulkan "
                          "Error Code : [{}]",
                          result));
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr; // Optional
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1;              // Optional

  result = vkCreateGraphicsPipelines(logicalDevice.Handle, VK_NULL_HANDLE, 1,
                                     &pipelineInfo, nullptr, &graphicsPipeline);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Graphics Pipeline",
              fmt::format("Unable to create Vulkan Graphics Pipeline.\nVulkan "
                          "Error Code: [{}]",
                          result));
  }
}

void Vulkan::CreateRenderPass() {
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChain.GetSurfaceFormat().format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkResult result = vkCreateRenderPass(logicalDevice.Handle, &renderPassInfo,
                                       nullptr, &renderPass);
  if (result != VK_SUCCESS) {
    ShowError(
        "Vulkan Render Pass",
        fmt::format(
            "Unable to create Vulkan Render Pass.\nVulkan Error Code: [{}]",
            result));
  }
}

void Vulkan::CreateFramebuffer() {
  auto swapChainImageViews = swapChain.GetImageViews();
  auto swapChainExtent = swapChain.GetExtent();

  swapChainFramebuffers.resize(swapChainImageViews.size());
  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice.Handle, &framebufferInfo, nullptr,
                            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void Vulkan::CreateCommandPool() {
  VulkanQueueFamilyIndices queueFamilyIndices = physicalDevice.QueueFamilies;

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  poolInfo.flags = 0; // Optional

  VkResult result = vkCreateCommandPool(logicalDevice.Handle, &poolInfo, nullptr,
                                        &commandPool);
  if (result != VK_SUCCESS) {
    ShowError(
        "Vulkan Command Pool",
        fmt::format("Unable to create command pool.\nVulkan Error Code: [{}]",
                    result));
  }
}

void Vulkan::CreateCommandBuffers() {
  commandBuffers.resize(swapChainFramebuffers.size());
  VkExtent2D swapChainExtent = swapChain.GetExtent();

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  VkResult result = vkAllocateCommandBuffers(logicalDevice.Handle, &allocInfo,
                                             commandBuffers.data());
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Command Buffers",
              fmt::format(
                  "Unable to create command buffers.\nVulkan Error Code: [{}]",
                  result));
  }

  for (size_t i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
    beginInfo.pInheritanceInfo = nullptr;                           // Optional

    result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

    if (result != VK_SUCCESS) {
      ShowError("Vulkan Command Buffers",
                fmt::format(
                    "Unable to begin command buffers.\nVulkan Error Code: [{}]",
                    result));
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[i]);

    result = vkEndCommandBuffer(commandBuffers[i]);
    if (result != VK_SUCCESS) {
      ShowError(
          "Vulkan Command Buffers",
          fmt::format("Unable to end command buffers.\nVulkan Error Code: [{}]",
                      result));
    }
  }
}

void Vulkan::CreateSemaphores() {
  auto swapChainImages = swapChain.GetImages();

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkResult imgAvailResult =
        vkCreateSemaphore(logicalDevice.Handle, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]);
    VkResult renderFinResult =
        vkCreateSemaphore(logicalDevice.Handle, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]);

    VkResult fenceResult = vkCreateFence(logicalDevice.Handle, &fenceInfo,
                                         nullptr, &inFlightFences[i]);

    if (imgAvailResult != VK_SUCCESS || renderFinResult != VK_SUCCESS ||
        fenceResult != VK_SUCCESS) {
      ShowError(
          "Vulkan Semaphore/Fences",
          fmt::format("Creationg of Vulkan Semaphores & Fences failed with the "
                      "following error codes.\nImage Available Semaphore "
                      "Result: {}\nRender Finished Semaphore Result: {}\nFence "
                      "Result: [{}]",
                      imgAvailResult, renderFinResult, fenceResult));
    }
  }
}
