#include "Vulkan.hpp"

#include <SDL2/SDL_vulkan.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

#include "Camera.hpp"
#include "SimpleMessageBox.hpp"
#include "VkInitializers.hpp"
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
  rectangle = new CoffeeMaker::Primitives::Rectangle();
  triangle = new Triangle();
}

Vulkan::~Vulkan() {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using Synchronization = CoffeeMaker::Renderer::Vulkan::Synchronization;
  using Commands = CoffeeMaker::Renderer::Vulkan::Commands;
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;

  vkDeviceWaitIdle(LogicalDevice::GetLogicalDevice());

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(nullptr);
  }

  // Destroy upload commands
  vkDestroyCommandPool(LogicalDevice::GetLogicalDevice(), _uploadContext.commandPool, nullptr);
  vkDestroyFence(LogicalDevice::GetLogicalDevice(), _uploadContext.uploadFence, nullptr);

  CleanupSwapChain();
  delete triangle;
  delete rectangle;
  // delete suzanne;

  Synchronization::DestroySyncTools();
  Commands::DestroyCommandPool();
  VulkanShaderManager::CleanAllShaders();
  CoffeeMaker::Renderer::Vulkan::MemoryAllocator::DestroyAllocator();
  LogicalDevice::Destroy();
  PhysicalDevice::ClearAllPhysicalDevices();
  CoffeeMaker::Renderer::Vulkan::Surface::Destroy();
  vkDestroyInstance(vulkanInstance, nullptr);
}

void Vulkan::EditorUpdate() {
  ImGui::Begin("Vulkan Renderer");
  if (ImGui::BeginTabBar("Test Tabs", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Physical Device")) {
      Editor_PhysicalDeviceSelection();
      ImGui::Separator();
      if (selectedPhysicalDeviceIndex != 9999) {
        Editor_PhysicalDeviceInformation();
      }

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
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  if (ImGui::BeginListBox("#physicalDevices")) {
    auto physicalDevices = PhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);
    for (size_t i = 0; i < physicalDevices.size(); i++) {
      bool selected = false;
      if (selectedPhysicalDeviceIndex == 9999) {
        selected = *physicalDevices[i] == *PhysicalDevice::GetPhysicalDeviceInUse();
      } else {
        selected = selectedPhysicalDeviceIndex == i;
      }
      if (ImGui::Selectable(physicalDevices[i]->Name(), selected)) {
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
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;

  auto physicalDevices = PhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);
  PhysicalDevice *selectedDevice = physicalDevices[selectedPhysicalDeviceIndex];

  if (ImGui::CollapsingHeader("Supported Extensions")) {
    if (ImGui::BeginTable("Supported Extensions", 2, ImGuiTableFlags_ScrollY, ImVec2(0.f, 300.f), 300.f)) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Extension Name");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("Spec Version");

      for (auto &supportedExtension : selectedDevice->SupportedExtensions) {
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
    ImGui::BulletText("API Version: %d", selectedDevice->Properties.apiVersion);
    ImGui::BulletText("Driver Version: %d", selectedDevice->Properties.driverVersion);
    ImGui::BulletText("Vendor ID: %d", selectedDevice->Properties.vendorID);
    ImGui::BulletText("Device ID: %d", selectedDevice->Properties.deviceID);
    ImGui::BulletText("Device Name: %s", selectedDevice->Properties.deviceName);
    ImGui::BulletText("Pipeline Cache UUID: %s", selectedDevice->Properties.pipelineCacheUUID);
  }

  if (ImGui::CollapsingHeader("Device Features")) {
    // NOTE: Set to address of the first feature.
    VkBool32 *pFeature = &selectedDevice->Features.robustBufferAccess;
    for (size_t i = 0; i < sizeof(selectedDevice->Features) / sizeof(VkBool32); i++) {
      ImGui::BulletText("%s: %s", features[i], *pFeature == 1 ? "On" : "Off");
      pFeature++;
    }
  }

  if (ImGui::CollapsingHeader("Swapchain Support")) {
    for (auto presentMode : selectedDevice->SwapChainSupport.presentModes) {
      std::string str = fmt::format("{}", VkPresentModeKHRString(presentMode));
      ImGui::Checkbox(str.c_str(), &selectedPresentMode);
    }
  }
}

Vulkan *Vulkan::GetRenderer() { return _mainRenderer; }

void Vulkan::CleanupSwapChain() {
  CoffeeMaker::Renderer::Vulkan::Framebuffer::ClearFramebuffers();
  CoffeeMaker::Renderer::Vulkan::Commands::FreeCommandBuffers();
  CoffeeMaker::Renderer::Vulkan::RenderPass::Destroy();
  CoffeeMaker::Renderer::Vulkan::Swapchain::Destroy();
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
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  vkDeviceWaitIdle(CoffeeMaker::Renderer::Vulkan::LogicalDevice::GetLogicalDevice());

  CleanupSwapChain();

  CoffeeMaker::Renderer::Vulkan::PhysicalDevice::GetPhysicalDeviceInUse()->QuerySwapchainSupport();
  CoffeeMaker::Renderer::Vulkan::Swapchain::CreateSwapchain();
  CoffeeMaker::Renderer::Vulkan::RenderPass::CreateRenderPass();
  CoffeeMaker::Renderer::Vulkan::Framebuffer::CreateFramebuffers();
  CoffeeMaker::Renderer::Vulkan::Commands::CreateCommandBuffers();

  Camera::SetMainCameraDimensions(
      PhysicalDevice::GetPhysicalDeviceInUse()->SwapChainSupport.capabilities.currentExtent.width,
      PhysicalDevice::GetPhysicalDeviceInUse()->SwapChainSupport.capabilities.currentExtent.height);
}

void BeginRender() {
  // TODO: Start render process here...
}

void EndRender() {
  // TODO: End render process here...
}

void Vulkan::Draw() {
  using Synchronization = CoffeeMaker::Renderer::Vulkan::Synchronization;
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;
  using Commands = CoffeeMaker::Renderer::Vulkan::Commands;

  triangle->Update();

  Synchronization::WaitForFence(currentFrame);
  Synchronization::ResetFence(currentFrame);

  ImGui::Render();

  uint32_t imageIndex;
  VkResult nxtImageResult =
      vkAcquireNextImageKHR(LogicalDevice::GetLogicalDevice(), Swapchain::GetVkpSwapchain(), 1000000000,
                            Synchronization::imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  // Check if a previous frame is using this image (i.e. there is its fence to wait on)
  if (Synchronization::imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
#ifdef COFFEEMAKER_STANDARD_WAIT_FOR_FENCE
    // NOTE: This is what the line should be for all systems, with timeout set to UINT32_MAX
    VkResult r = vkWaitForFences(LogicalDevice::GetLogicalDevice(), 1, &Synchronization::imagesInFlight[imageIndex],
                                 VK_TRUE, UINT32_MAX);
#else
    // TODO: Figure out why on Linux w/ integrated chips, this will deadlock the whole application.
    VkResult r =
        vkWaitForFences(LogicalDevice::GetLogicalDevice(), 1, &Synchronization::imagesInFlight[imageIndex], VK_TRUE, 0);
#endif
    if (r != VK_SUCCESS && r != VK_TIMEOUT) {
      std::cout << r << std::endl;
      abort();
    }
  }
  // Mark the image as now being in use by this frame
  Synchronization::imagesInFlight[imageIndex] = Synchronization::inFlightFences[currentFrame];
  // syncUtils.RenderFence2(currentFrame);

  if (nxtImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
  } else if (nxtImageResult != VK_SUCCESS && nxtImageResult != VK_SUBOPTIMAL_KHR) {
    SimpleMessageBox::ShowError("Drawing Error", fmt::format("Vulkan Error Code: [{}]", nxtImageResult));
  }

  Commands::CurrentCmdBufferIndex = imageIndex;
  Commands::ResetCommandBuffers(imageIndex);

  Commands::BeginRecording(imageIndex);
  // vkCmd* stuff...

  triangle->Draw();
  rectangle->Draw();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Commands::GetCurrentBuffer());
  Commands::EndRecording(imageIndex);

  VkCommandBuffer cmd = Commands::GetCurrentBuffer();

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &Synchronization::imageAvailableSemaphores[currentFrame];

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &Synchronization::imageRenderSemaphores[currentFrame];

  vkResetFences(LogicalDevice::GetLogicalDevice(), 1, &Synchronization::inFlightFences[currentFrame]);
  vkQueueSubmit(LogicalDevice::GraphicsQueue, 1, &submitInfo, Synchronization::inFlightFences[currentFrame]);

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &Synchronization::imageRenderSemaphores[currentFrame];

  std::array<VkSwapchainKHR, 1> swapChains{Swapchain::GetVkpSwapchain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains.data();
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;  // Optional

  VkResult presentResult = vkQueuePresentKHR(LogicalDevice::PresentQueue, &presentInfo);

  if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
    framebufferResized = false;
    RecreateSwapChain();
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

  for (auto &supportedExtension : supportedExtensions) {
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

void Vulkan::ShowError(const std::string &title, const std::string &message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), windowHandle);
  exit(1);
}

bool Vulkan::CheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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

  VULKAN_LAYERS_COUNT = 2;
  VULKAN_LAYERS.push_back("VK_LAYER_KHRONOS_validation");
  VULKAN_LAYERS.push_back("VK_LAYER_KHRONOS_synchronization2");
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
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;

  auto devices = PhysicalDevice::EnumeratePhysicalDevices(vulkanInstance);

  for (auto device : devices) {
    if (IsDeviceSuitable(device)) {
      PhysicalDevice::SelectPhysicalDevice(device->id);
      AddRequiredDeviceExtensionSupport(device->vkpPhysicalDevice);
      return;
    }
  }

  // if (physicalDevice.Handle == VK_NULL_HANDLE) {
  //   ShowError("Vulkan Physical Device", "Unable to find suitable GPU.");
  // }
}

bool Vulkan::IsDeviceSuitable(CoffeeMaker::Renderer::Vulkan::PhysicalDevice *device) {
  bool swapChainAdequate = false;
  bool extensionsSupported = device->AreExtensionsSupported(deviceExtensions);
  if (extensionsSupported) {
    swapChainAdequate = !device->SwapChainSupport.formats.empty() && !device->SwapChainSupport.presentModes.empty();
  }

#define FORCE_INTEGRATED_GPU
#ifdef FORCE_INTEGRATED_GPU
  bool result =
      device->QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device->IsIntegratedGPU();
#else
  bool result =
      device->QueueFamilies.IsComplete() && extensionsSupported && swapChainAdequate && device->IsDiscreteGPU();
#endif

  if (result) {
    fmt::print("Using device: {}\n", device->Name());
  }

  return result;
}

void Vulkan::CreateLogicalDevice() {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  LogicalDevice::SetExentions(deviceExtensions);
  LogicalDevice::SetLayers(VULKAN_LAYERS);
  LogicalDevice::CreateLogicalDevice(true);

  VulkanShaderManager::AssignLogicalDevice(LogicalDevice::GetLogicalDevice());
}

void Vulkan::CreateMemoryAllocator() {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  MemAlloc::CreateAllocator(PhysicalDevice::GetVkpPhysicalDeviceInUse(), LogicalDevice::GetLogicalDevice(),
                            vulkanInstance);
}

void Vulkan::CreateSurface() { CoffeeMaker::Renderer::Vulkan::Surface::CreateSurface(windowHandle, vulkanInstance); }

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
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;

  Swapchain::CreateSwapchain();

  Camera::SetMainCameraDimensions(Swapchain::GetSwapchain()->extent.width, Swapchain::GetSwapchain()->extent.height);
}

void Vulkan::CreateRenderPass() { CoffeeMaker::Renderer::Vulkan::RenderPass::CreateRenderPass(); }

void Vulkan::CreateFramebuffer() { CoffeeMaker::Renderer::Vulkan::Framebuffer::CreateFramebuffers(); }

void Vulkan::CreateCommands(bool recreation) {
  CoffeeMaker::Renderer::Vulkan::Commands::CreateCommandPool();
  CoffeeMaker::Renderer::Vulkan::Commands::CreateCommandBuffers();
}

void Vulkan::CreateUploadCommands() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VkCommandPoolCreateInfo uploadCommandPoolInfo =
      VkInit::CommandPoolCreateInfo(PhysicalDevice::GetPhysicalDeviceInUse()->QueueFamilies.graphicsFamily.value());
  vkCreateCommandPool(LogicalDevice::GetLogicalDevice(), &uploadCommandPoolInfo, nullptr, &_uploadContext.commandPool);
}

void Vulkan::CreateSemaphores() { CoffeeMaker::Renderer::Vulkan::Synchronization::CreateSyncTools(); }

void Vulkan::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::CommandBufferAllocateInfo(_uploadContext.commandPool, 1);

  VkCommandBuffer cmd;

  vkAllocateCommandBuffers(LogicalDevice::GetLogicalDevice(), &cmdAllocInfo, &cmd);

  VkCommandBufferBeginInfo cmdBeginInfo = VkInit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  function(cmd);

  vkEndCommandBuffer(cmd);

  VkSubmitInfo submit = VkInit::SubmitInfo(&cmd);
  vkQueueSubmit(LogicalDevice::GraphicsQueue, 1, &submit, _uploadContext.uploadFence);
  vkWaitForFences(LogicalDevice::GetLogicalDevice(), 1, &_uploadContext.uploadFence, true, 9999999999);
  vkResetFences(LogicalDevice::GetLogicalDevice(), 1, &_uploadContext.uploadFence);
  vkResetCommandPool(LogicalDevice::GetLogicalDevice(), _uploadContext.commandPool, 0);
}

void Vulkan::InitSyncStructures() {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VkFenceCreateInfo uploadFenceCreateInfo = VkInit::FenceCreateInfo();
  vkCreateFence(LogicalDevice::GetLogicalDevice(), &uploadFenceCreateInfo, nullptr, &_uploadContext.uploadFence);
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
