#include "Vulkan.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

#include "VulkanShaderManager.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

int Vulkan::MAX_FRAMES_IN_FLIGHT = 2;

Vulkan::Vulkan(const std::string &applicationName, SDL_Window *window) :
    enableValidationLayers(true),
    vulkanInstanceInitialized(false),
    windowHandle(window),
    vulkanAppInfo({}),
    vulkanInstanceCreateInfo({}),
    framebufferResized(false),
    framecount(0) {
  vulkanAppInfo.pApplicationName = applicationName.c_str();
  vulkanAppInfo.pEngineName = applicationName.c_str();

  SetupValidationLayers();
  InitVulkan();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateMemoryAllocator();
  CreateSwapChain();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffer();
  CreateCommands();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSemaphores();
  triangle = new Triangle(allocator, logicalDevice.Handle, renderPass.Handle, commands.GetBuffer(), &swapChain);
}

Vulkan::~Vulkan() {
  vkDeviceWaitIdle(logicalDevice.Handle);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(nullptr);
  }

  CleanupSwapChain();
  delete triangle;

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(logicalDevice.Handle, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(logicalDevice.Handle, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(logicalDevice.Handle, inFlightFences[i], nullptr);
  }
  syncUtils.DestroyHandles();
  commands.DestroyPool();
  vkDestroyCommandPool(logicalDevice.Handle, commandPool, nullptr);
  VulkanShaderManager::CleanAllShaders();
  vmaDestroyAllocator(allocator);
  logicalDevice.DestroyHandle();
  if (vulkanInstanceInitialized) {
    vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
  }
}

void Vulkan::CleanupSwapChain() {
  framebuffer.ClearFramebufferHandles();
  commands.FreeCommandBuffers();
  vkFreeCommandBuffers(logicalDevice.Handle, commandPool, static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  vkDestroyPipeline(logicalDevice.Handle, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(logicalDevice.Handle, pipelineLayout, nullptr);
  renderPass.DestroyHandle();
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
  vkWaitForFences(logicalDevice.Handle, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult nxtImageResult = vkAcquireNextImageKHR(logicalDevice.Handle, swapChain.Handle, UINT64_MAX,
                                                  imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (nxtImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return;
  } else if (nxtImageResult != VK_SUCCESS && nxtImageResult != VK_SUBOPTIMAL_KHR) {
    ShowError("Vulkan Draw",
              fmt::format("Vulkan Draw error occured, could not acquire next image.\n[{}]", nxtImageResult));
  }

  if (nxtImageResult != VK_SUCCESS) {
    ShowError("Vulkan Draw",
              fmt::format("Vulkan Draw error occured, could not acquire next image.\n[{}]", nxtImageResult));
  }

  if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(logicalDevice.Handle, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  imagesInFlight[imageIndex] = inFlightFences[currentFrame];

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(logicalDevice.Handle, 1, &inFlightFences[currentFrame]);
  VkResult result = vkQueueSubmit(logicalDevice.GraphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
    std::cerr << "Recreating swap chain..." << std::endl;
    framebufferResized = false;
    RecreateSwapChain();
  } else if (result != VK_SUCCESS) {
    ShowError("Vulkan Queue Submit", fmt::format("Failed to submit draw command to buffer: [{}]", result));
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  std::array<VkSwapchainKHR, 1> swapChains{swapChain.Handle};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains.data();
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;  // Optional

  vkQueuePresentKHR(logicalDevice.PresentQueue, &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::Draw2() {
  syncUtils.WaitForFence();
  syncUtils.ResetFence();

  // NOTE: Request image from the swapchain, one second timeout
  uint32_t imageIndex;
  VkResult nxtImageResult = vkAcquireNextImageKHR(logicalDevice.Handle, swapChain.Handle, 1000000000,
                                                  syncUtils.PresentSemaphore(), VK_NULL_HANDLE, &imageIndex);

  if (nxtImageResult != VK_SUCCESS) {
    SimpleMessageBox::ShowError("Drawing Error", fmt::format("Vulkan Error Code: [{}]", nxtImageResult));
  }

  commands.ResetCommandBuffers();
  float flash = std::abs(std::sin(framecount / 120.f));
  commands.SetRenderClearColor({0.0f, 0.0f, flash, 1.0f});

  commands.BeginRecording(imageIndex);
  // vkCmd* stuff...
  triangle->Draw();
  commands.EndRecording();

  VkSemaphore presentSemaRef = syncUtils.PresentSemaphore();
  VkSemaphore renderSemaRef = syncUtils.RenderSemaphore();
  VkCommandBuffer cmd = commands.GetBuffer();

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &presentSemaRef;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &renderSemaRef;

  vkQueueSubmit(logicalDevice.GraphicsQueue, 1, &submitInfo, syncUtils.RenderFence());

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderSemaRef;

  std::array<VkSwapchainKHR, 1> swapChains{swapChain.Handle};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains.data();
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;  // Optional

  vkQueuePresentKHR(logicalDevice.PresentQueue, &presentInfo);
  framecount++;
}

void Vulkan::InitVulkan() {
  // Check for Layer Support
  CheckValidationLayerSupport();

  // Get all supported vulkan instance extensions
  uint32_t extensionSupportCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionSupportCount, nullptr);
  std::vector<VkExtensionProperties> supportedExtensions(extensionSupportCount);
  std::vector<const char *> supportedExtensionNames{};
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionSupportCount, supportedExtensions.data());

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
      ShowError("Vulkan Init Error", fmt::format("Specified extension is NOT supported by this "
                                                 "machine\n Vulkan Extension Requested: [{}]",
                                                 requiredExtension));
    }
  }

  // Instance Creation Info
  vulkanInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vulkanInstanceCreateInfo.pApplicationInfo = &vulkanAppInfo;
  vulkanInstanceCreateInfo.enabledLayerCount = 0;
  vulkanInstanceCreateInfo.ppEnabledLayerNames = nullptr;
  vulkanInstanceCreateInfo.enabledExtensionCount = REQUIRED_VULKAN_EXTENSIONS_COUNT;
  vulkanInstanceCreateInfo.ppEnabledExtensionNames = REQUIRED_VULKAN_EXTENSIONS.data();
  vulkanInstanceCreateInfo.pNext = nullptr;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers) {
    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    vulkanInstanceCreateInfo.enabledLayerCount = VULKAN_LAYERS.size();
    vulkanInstanceCreateInfo.ppEnabledLayerNames = VULKAN_LAYERS.data();
    vulkanInstanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  }

  VkResult result = vkCreateInstance(&vulkanInstanceCreateInfo, nullptr, &vulkanInstance);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Init Error", fmt::format("Error resulted in creating vulkan instance\n Vulkan "
                                               "Error Code: [{}]",
                                               result));
  }
  vulkanInstanceInitialized = true;
}

VkInstance Vulkan::InstanceHandle() const { return vulkanInstance; }

void Vulkan::ShowError(const std::string &title, const std::string &message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), windowHandle);
  exit(1);
}

bool Vulkan::CheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::cout << "Supported Layers" << std::endl;
  for (auto &availableLayer : availableLayers) {
    fmt::print("Name: {}\nDescription: {}\n", availableLayer.layerName, availableLayer.description);
  }

  for (const char *layerName : VULKAN_LAYERS) {
    bool layerFound = false;
    for (size_t i = 0; i < availableLayers.size(); ++i) {
      if (strcmp(layerName, availableLayers[i].layerName) == 0) {
        layerFound = true;
      }
    }
    if (!layerFound) {
      fmt::print(
          "Specified Layer is NOT supported by this "
          "machine\n Vulkan Layer Requested: [{}]\n",
          layerName);
      ShowError("Vulkan Init Error", fmt::format("Specified Layer is NOT supported by this "
                                                 "machine\n Vulkan Layer Requested: [{}]",
                                                 layerName));
    }
  }

  return true;
}

void Vulkan::GetRequiredExtensions() {
  // SDL Required Extensions
  SDL_Vulkan_GetInstanceExtensions(windowHandle, &REQUIRED_VULKAN_EXTENSIONS_COUNT, nullptr);
  REQUIRED_VULKAN_EXTENSIONS.resize(REQUIRED_VULKAN_EXTENSIONS_COUNT);
  SDL_Vulkan_GetInstanceExtensions(windowHandle, &REQUIRED_VULKAN_EXTENSIONS_COUNT, REQUIRED_VULKAN_EXTENSIONS.data());

  // Debug utils
  if (enableValidationLayers) {
    REQUIRED_VULKAN_EXTENSIONS.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    REQUIRED_VULKAN_EXTENSIONS_COUNT++;
  }
  REQUIRED_VULKAN_EXTENSIONS.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
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

void Vulkan::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;  // Optional user data
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

VkResult Vulkan::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator) {
  auto func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(vulkanInstance, pCreateInfo, pAllocator, &debugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void Vulkan::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks *pAllocator) {
  auto func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
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

bool Vulkan::IsDeviceSuitable(VulkanPhysicalDevice &device) {
  bool swapChainAdequate = false;
  bool extensionsSupported = device.AreExtensionsSupported(deviceExtensions);
  if (extensionsSupported) {
    device.QuerySwapChainSupport();
    swapChainAdequate = !device.SwapChainSupport.formats.empty() && !device.SwapChainSupport.presentModes.empty();
  }

  fmt::print("Checking device: {}\n", device.ToString());

  bool result = device.QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device.IsDiscreteGPU();

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
  VulkanShaderManager::AssignLogicalDevice(logicalDevice.Handle);
}

void Vulkan::CreateMemoryAllocator() {
  VmaAllocatorCreateInfo info{};
  info.physicalDevice = physicalDevice.Handle;
  info.device = logicalDevice.Handle;
  info.instance = vulkanInstance;
  vmaCreateAllocator(&info, &allocator);
}

void Vulkan::CreateSurface() {
  if (!SDL_Vulkan_CreateSurface(windowHandle, vulkanInstance, &vulkanSurface)) {
    ShowError("SDL Vulkan Surface",
              fmt::format("SDL2 was unable to create Vulkan Surface.\nSDL Error: [{}]", SDL_GetError()));
  }
}

void Vulkan::AddRequiredDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
  for (auto availExt : availableExtensions) {
    if (strcmp(availExt.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0) {
      // if (availExt.extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) {
      /**
       * If a physical device allows for this extension, then it MUST be enabled.
       * @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_portability_subset.html
       */
      deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
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

  VkShaderModule fragShaderModule = VulkanShaderManager::CreateShaderModule(logicalDevice.Handle, fragShaderCode);
  VkShaderModule vertShaderModule = VulkanShaderManager::CreateShaderModule(logicalDevice.Handle, vertShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{vertShaderStageInfo, fragShaderStageInfo};

  SetupFixedFunctionsPipeline(shaderStages);

  VulkanShaderManager::CleanupShaderModule(logicalDevice.Handle, fragShaderModule);
  VulkanShaderManager::CleanupShaderModule(logicalDevice.Handle, vertShaderModule);
}

void Vulkan::CreateRenderPass() {
  renderPass.SetLogicalDevice(&logicalDevice);
  renderPass.SetSwapchain(&swapChain);
  renderPass.Build();
}

void Vulkan::CreateFramebuffer() {
  framebuffer.SetLogicalDevice(&logicalDevice);
  framebuffer.SetRenderPass(&renderPass);
  framebuffer.SetSwapchain(&swapChain);
  framebuffer.Build();
}

void Vulkan::CreateCommands() {
  commands.SetLogicalDevice(&logicalDevice);
  commands.SetSwapchain(&swapChain);
  commands.SetFramebuffers(&framebuffer);
  commands.SetRenderPass(&renderPass);
  commands.SetPhysicalDevice(&physicalDevice);
  commands.CreateCommandPool();
  commands.CreateCommandBuffers();
}

void Vulkan::CreateCommandPool() {
  VulkanQueueFamilyIndices queueFamilyIndices = physicalDevice.QueueFamilies;

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  poolInfo.flags = 0;  // Optional

  VkResult result = vkCreateCommandPool(logicalDevice.Handle, &poolInfo, nullptr, &commandPool);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Command Pool", fmt::format("Unable to create command pool.\nVulkan Error Code: [{}]", result));
  }
}

void Vulkan::CreateCommandBuffers() {
  commandBuffers.resize(framebuffer.FramebufferHandles.size());
  VkExtent2D swapChainExtent = swapChain.GetExtent();

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  VkResult result = vkAllocateCommandBuffers(logicalDevice.Handle, &allocInfo, commandBuffers.data());
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Command Buffers",
              fmt::format("Unable to create command buffers.\nVulkan Error Code: [{}]", result));
  }

  for (size_t i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;  // Optional
    beginInfo.pInheritanceInfo = nullptr;                            // Optional

    result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

    if (result != VK_SUCCESS) {
      ShowError("Vulkan Command Buffers",
                fmt::format("Unable to begin command buffers.\nVulkan Error Code: [{}]", result));
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.Handle;
    renderPassInfo.framebuffer = framebuffer.FramebufferHandles[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[i]);

    result = vkEndCommandBuffer(commandBuffers[i]);
    if (result != VK_SUCCESS) {
      ShowError("Vulkan Command Buffers",
                fmt::format("Unable to end command buffers.\nVulkan Error Code: [{}]", result));
    }
  }
}

void Vulkan::CreateSemaphores() {
  syncUtils.SetLogicalDevice(&logicalDevice);
  syncUtils.CreateSemaphores();
  syncUtils.CreateRenderFence();
  syncUtils.Build();

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
        vkCreateSemaphore(logicalDevice.Handle, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
    VkResult renderFinResult =
        vkCreateSemaphore(logicalDevice.Handle, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);

    VkResult fenceResult = vkCreateFence(logicalDevice.Handle, &fenceInfo, nullptr, &inFlightFences[i]);

    if (imgAvailResult != VK_SUCCESS || renderFinResult != VK_SUCCESS || fenceResult != VK_SUCCESS) {
      ShowError("Vulkan Semaphore/Fences", fmt::format("Creationg of Vulkan Semaphores & Fences failed with the "
                                                       "following error codes.\nImage Available Semaphore "
                                                       "Result: {}\nRender Finished Semaphore Result: {}\nFence "
                                                       "Result: [{}]",
                                                       imgAvailResult, renderFinResult, fenceResult));
    }
  }
}
