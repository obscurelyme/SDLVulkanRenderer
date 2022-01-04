#include "Renderer/Vulkan/Core.hpp"

CoffeeMaker::Renderer::Vulkan::VulkanRenderer *CoffeeMaker::Renderer::Vulkan::VulkanRenderer::renderInstance{nullptr};

CoffeeMaker::Renderer::Vulkan::VulkanRenderer::VulkanRenderer(const std::string &applicationName, SDL_Window *window) {
  if (renderInstance != nullptr) {
    SDL_LogError(0, "Could not instantiate multiple instances of VulkanRenderer");
    exit(10000);
  }

  vulkanAppInfo.pApplicationName = applicationName.c_str();
  vulkanAppInfo.pEngineName = applicationName.c_str();

  SetupValidationLayers();
  InitVulkan();
  SetupDebugMessenger();

  PickPhysicalDevice();

  renderInstance = this;
}

CoffeeMaker::Renderer::Vulkan::VulkanRenderer::~VulkanRenderer() {
  vkDeviceWaitIdle(LogicalDevice::GetLogicalDevice());

  // if (enableValidationLayers) {
  //   DestroyDebugUtilsMessengerEXT(nullptr);
  // }

  // Destroy upload commands
  // vkDestroyFence(logicalDevice.Handle, _uploadContext._uploadFence, nullptr);

  // CleanupSwapChain();

  Synchronization::DestroySyncTools();
  Commands::DestroyCommandPool();
  // VulkanShaderManager::CleanAllShaders();
  MemoryAllocator::DestroyAllocator();
  LogicalDevice::Destroy();
  PhysicalDevice::ClearAllPhysicalDevices();
  Surface::Destroy();
  vkDestroyInstance(vulkanInstance, nullptr);
}

CoffeeMaker::Renderer::Vulkan::VulkanRenderer *CoffeeMaker::Renderer::Vulkan::VulkanRenderer::GetRenderer() {
  return renderInstance;
}

void CoffeeMaker::Renderer::Vulkan::VulkanRenderer::CleanSwapchain() {
  Framebuffer::CreateFramebuffers();
  Commands::FreeCommandBuffers();
  RenderPass::Destroy();
  Swapchain::Destroy();
}

void CoffeeMaker::Renderer::Vulkan::VulkanRenderer::RecreateSwapchain() {}

void CoffeeMaker::Renderer::Vulkan::VulkanRenderer::Init() {
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
}
