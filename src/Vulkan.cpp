#include "Vulkan.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

#include "Camera.hpp"
#include "VkInitializers.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanShaderManager.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

std::string VkPresentModeKHRString(VkPresentModeKHR mode) {
  switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
      return "Immediate Mode";
    case VK_PRESENT_MODE_MAILBOX_KHR:
      return "Triple Buffering";
    case VK_PRESENT_MODE_FIFO_KHR:
      return "Vertical Sync";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
      return "Relaxed Vertical Sync";
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
      return "Shared Demand";
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
      return "Shared Continuous";
    case VK_PRESENT_MODE_MAX_ENUM_KHR:
      return "Max Enum (Do not set to this value)";
    default:
      return "Unknown Vulkan Present Mode";
  }
}

int Vulkan::MAX_FRAMES_IN_FLIGHT = 2;

Vulkan *Vulkan::_mainRenderer = nullptr;

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
  CreateFramebuffer();
  CreateCommands();
  CreateUploadCommands();
  CreateSemaphores();
  InitSyncStructures();
  _mainRenderer = this;
  triangle = new Triangle(logicalDevice.Handle, renderPass.Handle, &commands, &swapChain);
  // suzanne = new Suzanne(allocator, logicalDevice.Handle, renderPass.Handle, commands.GetBuffer(), &swapChain);
}

Vulkan::~Vulkan() {
  vkDeviceWaitIdle(logicalDevice.Handle);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(nullptr);
  }

  // Destroy upload commands
  vkDestroyCommandPool(logicalDevice.Handle, _uploadContext._commandPool, nullptr);
  vkDestroyFence(logicalDevice.Handle, _uploadContext._uploadFence, nullptr);

  CleanupSwapChain();
  delete triangle;
  // delete suzanne;

  syncUtils.DestroyHandles();
  commands.DestroyPool();
  VulkanShaderManager::CleanAllShaders();
  VulkanAllocator::DestroyAllocator();
  logicalDevice.DestroyHandle();
  if (vulkanInstanceInitialized) {
    vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
  }
}

void Vulkan::EditorUpdate() {
  ImGui::Begin("Vulkan Renderer");
  if (ImGui::BeginTabBar("Test Tabs", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Physical Device")) {
      Editor_PhysicalDeviceSelection();
      ImGui::Separator();
      Editor_PhysicalDeviceInformation();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Logical Device")) {
      ImGui::Text("logical stuff...");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Swapchain")) {
      ImGui::Text("swapchain info...!");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Framebuffer")) {
      ImGui::Text("swapchain info...!");
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void Vulkan::Editor_PhysicalDeviceSelection() {
  if (ImGui::BeginListBox("#physicalDevices")) {
    auto physicalDevices = VulkanPhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);
    for (size_t i = 0; i < physicalDevices.size(); i++) {
      bool selected = false;
      if (selectedPhysicalDeviceIndex == 9999) {
        selected = physicalDevices[i] == physicalDevice;
      } else {
        selected = selectedPhysicalDeviceIndex == i;
      }
      if (ImGui::Selectable(physicalDevices[i].Name(), selected)) {
        selectedPhysicalDeviceIndex = i;
      }
      if (selected) {
        if (selectedPhysicalDeviceIndex == 9999) {
          selectedPhysicalDeviceIndex = i;
        }
        ImGui::SetItemDefaultFocus();
      }
    }

    ImGui::EndListBox();
  }
}

void Vulkan::Editor_PhysicalDeviceInformation() {
  auto physicalDevices = VulkanPhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);
  VulkanPhysicalDevice selectedDevice = physicalDevices[selectedPhysicalDeviceIndex];

  if (ImGui::CollapsingHeader("Supported Extensions")) {
    if (ImGui::BeginTable("Supported Extensions", 2, ImGuiTableFlags_ScrollY, ImVec2(0.f, 300.f), 300.f)) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Extension Name");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("Spec Version");

      for (auto &supportedExtension : selectedDevice.SupportedExtensions) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", supportedExtension.extensionName);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", std::to_string(supportedExtension.specVersion).c_str());
      }
      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Device Properties")) {
    ImGui::BulletText("API Version: %d", selectedDevice.Properties.apiVersion);
    ImGui::BulletText("Driver Version: %d", selectedDevice.Properties.driverVersion);
    ImGui::BulletText("Vendor ID: %d", selectedDevice.Properties.vendorID);
    ImGui::BulletText("Device ID: %d", selectedDevice.Properties.deviceID);
    ImGui::BulletText("Device Name: %s", selectedDevice.Properties.deviceName);
    ImGui::BulletText("Pipeline Cache UUID: %s", selectedDevice.Properties.pipelineCacheUUID);
  }

  if (ImGui::CollapsingHeader("Device Features")) {
    // NOTE: Set to address of the first feature.
    VkBool32 *pFeature = &selectedDevice.Features.robustBufferAccess;
    for (size_t i = 0; i < sizeof(selectedDevice.Features) / sizeof(VkBool32); i++) {
      ImGui::BulletText("%s: %s", features[i], *pFeature == 1 ? "On" : "Off");
      pFeature++;
    }
  }

  if (ImGui::CollapsingHeader("Swapchain Support")) {
    for (auto presentMode : selectedDevice.SwapChainSupport.presentModes) {
      std::string str = fmt::format("{}", VkPresentModeKHRString(presentMode));
      ImGui::Checkbox(str.c_str(), &selectedPresentMode);
    }
  }
}

Vulkan *Vulkan::GetRenderer() { return _mainRenderer; }

void Vulkan::CleanupSwapChain() {
  framebuffer.ClearFramebufferHandles();
  commands.FreeCommandBuffers();
  // TODO: Notify all pipelines and layouts to be destroyed...
  EmitSwapChainWillBeDestroyed();
  triangle->OnSwapChainDestroyed();
  renderPass.DestroyHandle();
  swapChain.DestroyHandle();
  std::cout << "Cleaned up swap chain" << std::endl;
}

/**
 * @brief Non-negotiably recreate the swapchain.
 * Typically, tutorials will have one set a flag on window resize and wait for a non VK_SUCCESS
 * result from either vkAcquireNextImageKHR or vkQueuePresentKHR, and then recreate the swapchain.
 * There can be an odd issue with this with SDL2 in that the window surface is still being used
 * due to the resize taking place, and in addition to this we are trying to change up a swapchain.
 * The result is an odd race condition where you can see VK_ERROR_NATIVE_WINDOW_IN_USE_KHR.
 * The result is a failure to create the swapchain with the new window size. So in order to get
 * around this complete is to just recreate the swapchain, no questions asked.
 */
void Vulkan::FramebufferResize() { RecreateSwapChain(); }

void Vulkan::RecreateSwapChain() {
  vkDeviceWaitIdle(logicalDevice.Handle);

  CleanupSwapChain();

  physicalDevice.QuerySwapChainSupport();
  CreateSwapChain();
  CreateRenderPass();
  CreateFramebuffer();
  CreateCommands(true);
  // TODO: Nofity all pipelines and layouts to be created...
  EmitSwapChainCreated();
  triangle->OnSwapChainRecreated(renderPass.Handle, &commands, &swapChain);
  Camera::SetMainCameraDimensions(physicalDevice.SwapChainSupport.capabilities.currentExtent.width,
                                  physicalDevice.SwapChainSupport.capabilities.currentExtent.height);
}

void Vulkan::Draw() {
  triangle->Update();
  std::cout << "Waiting for fence at " << currentFrame << " " << syncUtils.RenderFence2(currentFrame) << std::endl;
  syncUtils.WaitForFence2(currentFrame);
  syncUtils.ResetFence2(currentFrame);
  std::cout << "Fence reset for " << currentFrame << " " << syncUtils.RenderFence2(currentFrame) << std::endl;

  ImGui::Render();

  // NOTE: Request image from the swapchain, one second timeout
  uint32_t imageIndex;
  std::cout << "Getting image...." << std::endl;
  VkResult nxtImageResult =
      vkAcquireNextImageKHR(logicalDevice.Handle, swapChain.Handle, 1000000000,
                            syncUtils.PresentSemaphore2(currentFrame), VK_NULL_HANDLE, &imageIndex);
  std::cout << "Got image" << std::endl;

  std::cout << "Image In Flight Value: " << syncUtils.imagesInFlight[imageIndex] << std::endl;
  // Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (syncUtils.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    std::cout << "Waiting for image in flight: " << imageIndex << " " << syncUtils.imagesInFlight[imageIndex]
              << std::endl;
    vkWaitForFences(logicalDevice.Handle, 1, &syncUtils.imagesInFlight[imageIndex], VK_TRUE, 1000000000);
    std::cout << "Done waiting for image in flight: " << imageIndex << std::endl;
  }
  // Mark the image as now being in use by this frame
  std::cout << "Marking image in flight: " << imageIndex << " to frame " << currentFrame << std::endl;
  syncUtils.imagesInFlight[imageIndex] = syncUtils.RenderFence2(currentFrame);
  std::cout << "Marked image in flight: " << imageIndex << " to fence " << syncUtils.imagesInFlight[imageIndex]
            << std::endl;

  if (nxtImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    std::cout << "Implicit Recreation" << std::endl;
    RecreateSwapChain();
    std::cout << "Implicit Recreation Done" << std::endl;
  } else if (nxtImageResult != VK_SUCCESS) {
    SimpleMessageBox::ShowError("Drawing Error", fmt::format("Vulkan Error Code: [{}]", nxtImageResult));
  }

  commands.CurrentCmdBufferIndex = imageIndex;
  std::cout << "Using command buffers at " << imageIndex << std::endl;
  commands.ResetCommandBuffers2(imageIndex);
  float flash = std::abs(std::sin(framecount / 120.f));
  commands.SetRenderClearColor({0.0f, 0.0f, flash, 1.0f});

  commands.BeginRecording(imageIndex);
  // vkCmd* stuff...
  triangle->Draw();
  // suzanne->Draw();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands.GetCurrentBuffer());
  commands.EndRecording(imageIndex);

  VkSemaphore presentSemaRef = syncUtils.PresentSemaphore2(currentFrame);
  VkSemaphore renderSemaRef = syncUtils.RenderSemaphore2(currentFrame);
  VkCommandBuffer cmd = commands.GetCurrentBuffer();

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

  std::cout << "Reset Fence before queuing " << currentFrame << " " << syncUtils.inFlightFences[currentFrame]
            << std::endl;
  vkResetFences(logicalDevice.Handle, 1, &syncUtils.inFlightFences[currentFrame]);
  std::cout << "Queuing..." << std::endl;
  vkQueueSubmit(logicalDevice.GraphicsQueue, 1, &submitInfo, syncUtils.RenderFence2(currentFrame));
  std::cout << "Queued" << std::endl;

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderSemaRef;

  std::array<VkSwapchainKHR, 1> swapChains{swapChain.Handle};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains.data();
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;  // Optional

  std::cout << "Presenting..." << std::endl;
  VkResult presentResult = vkQueuePresentKHR(logicalDevice.PresentQueue, &presentInfo);
  std::cout << "Presented" << std::endl;

  if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
    framebufferResized = false;
    std::cout << "Explicit Recreation" << std::endl;
    RecreateSwapChain();
    std::cout << "Explicit Recreation Done" << std::endl;
  } else if (presentResult != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
  framecount++;
  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;  // Forced alternating value of 0 and 1
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
  std::vector<VulkanPhysicalDevice> &devices = VulkanPhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);

  for (VulkanPhysicalDevice &device : devices) {
    device.SetSurface(vulkanSurface);
    device.FindQueueFamilies();
    device.QuerySwapChainSupport();
  }

  for (VulkanPhysicalDevice &device : devices) {
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
    // device.QuerySwapChainSupport();
    swapChainAdequate = !device.SwapChainSupport.formats.empty() && !device.SwapChainSupport.presentModes.empty();
  }

  fmt::print("Checking device: {}\n", device.ToString());

#define FORCE_INTEGRATED_GPU
#ifdef FORCE_INTEGRATED_GPU
  bool result =
      device.QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device.IsIntegratedGPU();
#else
  bool result = device.QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device.IsDiscreteGPU();
#endif

  if (result) {
    fmt::print("Using device: {}\n", device.Name());
  }

  for (auto e : device.SupportedExtensions) {
    std::cout << e.extensionName << std::endl;
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
  VulkanAllocator::CreateAllocator(physicalDevice.Handle, logicalDevice.Handle, vulkanInstance);
  allocator = VulkanAllocator::allocator;
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
  swapChain.SetWindow(windowHandle);
  swapChain.SetPhysicalDevice(&physicalDevice);
  swapChain.SetLogicalDevice(&logicalDevice);
  swapChain.ChooseSwapSurfaceFormat();
  swapChain.ChoosePresentationMode();
  swapChain.ChooseSwapExtent();
  swapChain.Create();
  swapChain.CreateImageViews();
  swapChain.CreateDepthImageView(allocator);
}

void Vulkan::CreateRenderPass() {
  renderPass.SetLogicalDevice(&logicalDevice);
  renderPass.SetSwapchain(&swapChain);
  renderPass.SetDepthFormat(swapChain.GetDepthFormat());
  renderPass.Build();
}

void Vulkan::CreateFramebuffer() {
  framebuffer.SetLogicalDevice(&logicalDevice);
  framebuffer.SetRenderPass(&renderPass);
  framebuffer.SetSwapchain(&swapChain);
  framebuffer.Build();
}

void Vulkan::CreateCommands(bool recreation) {
  commands.SetLogicalDevice(&logicalDevice);
  commands.SetSwapchain(&swapChain);
  commands.SetFramebuffers(&framebuffer);
  commands.SetRenderPass(&renderPass);
  commands.SetPhysicalDevice(&physicalDevice);
  if (!recreation) {
    commands.CreateCommandPool();
  }
  commands.CreateCommandBuffers();
}

void Vulkan::CreateUploadCommands() {
  VkCommandPoolCreateInfo uploadCommandPoolInfo =
      VkInit::CommandPoolCreateInfo(physicalDevice.QueueFamilies.graphicsFamily.value());
  vkCreateCommandPool(logicalDevice.Handle, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool);
}

void Vulkan::CreateSemaphores() {
  syncUtils.SetLogicalDevice(&logicalDevice);
  syncUtils.CreateSemaphores();
  syncUtils.CreateRenderFence();
  syncUtils.imagesInFlight.resize(swapChain.GetImageCount(), VK_NULL_HANDLE);
  syncUtils.Build();
  syncUtils.Build2();
}

void Vulkan::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
  VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::CommandBufferAllocateInfo(_uploadContext._commandPool, 1);

  VkCommandBuffer cmd;

  vkAllocateCommandBuffers(logicalDevice.Handle, &cmdAllocInfo, &cmd);

  VkCommandBufferBeginInfo cmdBeginInfo = VkInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  function(cmd);

  vkEndCommandBuffer(cmd);

  VkSubmitInfo submit = VkInit::SubmitInfo(&cmd);
  vkQueueSubmit(logicalDevice.GraphicsQueue, 1, &submit, _uploadContext._uploadFence);
  vkWaitForFences(logicalDevice.Handle, 1, &_uploadContext._uploadFence, true, 9999999999);
  vkResetFences(logicalDevice.Handle, 1, &_uploadContext._uploadFence);
  vkResetCommandPool(logicalDevice.Handle, _uploadContext._commandPool, 0);
}

void Vulkan::InitSyncStructures() {
  VkFenceCreateInfo uploadFenceCreateInfo = VkInit::FenceCreateInfo();
  vkCreateFence(logicalDevice.Handle, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence);
}

void Vulkan::EmitSwapChainWillBeDestroyed() {
  const size_t size = swapChainDestroyedListeners.size();

  for (size_t i = 0; i < size; i++) {
    swapChainDestroyedListeners[i]();
  }
}

void Vulkan::EmitSwapChainCreated() {
  const size_t size = swapChainCreatedListeners.size();

  for (size_t i = 0; i < size; i++) {
    swapChainCreatedListeners[i]();
  }
}

void Vulkan::AddSwapChainDestroyedListener(std::function<void(void)> fn) {
  _mainRenderer->swapChainDestroyedListeners.push_back(fn);
}

void Vulkan::AddSwapChainCreatedListener(std::function<void(void)> fn) {
  _mainRenderer->swapChainCreatedListeners.push_back(fn);
}
