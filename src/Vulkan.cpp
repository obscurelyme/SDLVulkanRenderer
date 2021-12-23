#include "Vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <set>
#include <vector>

Vulkan::Vulkan(const std::string &applicationName, SDL_Window *window)
    : enableValidationLayers(true), vulkanInstanceInitialized(false),
      windowHandle(window), vulkanAppInfo({}), vulkanInstanceCreateInfo({}),
      vulkanPhysicalDevice(VK_NULL_HANDLE) {
  vulkanAppInfo.pApplicationName = applicationName.c_str();
  vulkanAppInfo.pEngineName = applicationName.c_str();

  SetupValidationLayers();
  InitVulkan();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateImageViews();
  CreateGraphicsPipeline();
}

Vulkan::~Vulkan() {
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(nullptr);
  }
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(vulkanLogicalDevice, imageView, nullptr);
  }
  vkDestroySwapchainKHR(vulkanLogicalDevice, vulkanSwapChain, nullptr);
  vkDestroyDevice(vulkanLogicalDevice, nullptr);
  if (vulkanInstanceInitialized) {
    vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
    vkDestroyInstance(vulkanInstance, nullptr);
  }
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
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    ShowError("Vulkan Physical Device",
              "Unable to find GPUs with Vulkan support.");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

  // Selects the fist available suitable device
  for (const auto &device : devices) {
    if (IsDeviceSuitable(device)) {
      vulkanPhysicalDevice = device;
      break;
    }
  }

  if (vulkanPhysicalDevice == VK_NULL_HANDLE) {
    ShowError("Vulkan Physical Device", "Unable to find suitable GPU.");
  }
}

bool Vulkan::IsDeviceSuitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  fmt::print("Checking device: {}\ntype: {}\n", deviceProperties.deviceName,
             deviceProperties.deviceType);
  QueueFamilyIndices indices = FindQueueFamilies(device);

  bool swapChainAdequate = false;
  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  bool result =
      indices.IsComplete() && extensionsSupported && swapChainAdequate;

  if (result) {
    fmt::print("Using device: {}\n", deviceProperties.deviceName);
  }

  return result;
}

QueueFamilyIndices Vulkan::FindQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;
  // Assign index to queue families that could be found
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkanSurface,
                                         &presentSupport);

    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    if (presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.IsComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

void Vulkan::CreateLogicalDevice() {
  QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};
  float queuePriority = 1.0f;

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // VkPhysicalDeviceFeatures deviceFeatures{};
  // REQUIRED_VULKAN_DEVICE_EXTENSIONS.push_back(
  //     VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
  deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

  VkDeviceCreateInfo logicalDeviceCreateInfo{};
  logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  logicalDeviceCreateInfo.pNext = nullptr;
  logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  logicalDeviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  logicalDeviceCreateInfo.pEnabledFeatures = nullptr; // NOTE: use later
  logicalDeviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  logicalDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    logicalDeviceCreateInfo.enabledLayerCount = VULKAN_LAYERS.size();
    logicalDeviceCreateInfo.ppEnabledLayerNames = VULKAN_LAYERS.data();
  } else {
    logicalDeviceCreateInfo.enabledLayerCount = 0;
    logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr;
  }

  VkResult result =
      vkCreateDevice(vulkanPhysicalDevice, &logicalDeviceCreateInfo, nullptr,
                     &vulkanLogicalDevice);
  if (result != VK_SUCCESS) {
    ShowError(
        "Vulkan Logical Device",
        fmt::format(
            "Unable to create Vulkan Logical Device.\nVulkan Error Code: [{}]",
            result));
  }

  vkGetDeviceQueue(vulkanLogicalDevice, indices.graphicsFamily.value(), 0,
                   &vulkanGraphicsQueue);
  vkGetDeviceQueue(vulkanLogicalDevice, indices.presentFamily.value(), 0,
                   &vulkanPresentQueue);
}

void Vulkan::CreateSurface() {
  if (!SDL_Vulkan_CreateSurface(windowHandle, vulkanInstance, &vulkanSurface)) {
    ShowError("SDL Vulkan Surface",
              fmt::format(
                  "SDL2 was unable to create Vulkan Surface.\nSDL Error: [{}]",
                  SDL_GetError()));
  }
}

bool Vulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

SwapChainSupportDetails Vulkan::QuerySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanSurface,
                                            &details.capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount,
                                       nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface,
                                            &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, vulkanSurface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR Vulkan::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  // NOTE: settle on the first format as it most likely is "good enough"
  return availableFormats[0];
}

VkPresentModeKHR Vulkan::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode; // Leverage triple buffering if we can.
    }
  }

  // else, fallback to v-sync
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
Vulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    SDL_Vulkan_GetDrawableSize(windowHandle, &width, &height);
    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void Vulkan::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport =
      QuerySwapChainSupport(vulkanPhysicalDevice);

  VkSurfaceFormatKHR surfaceFormat =
      ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = vulkanSurface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(vulkanLogicalDevice, &createInfo,
                                         nullptr, &vulkanSwapChain);
  if (result != VK_SUCCESS) {
    ShowError("Vulkan Swap Chain",
              fmt::format("Unable to create the Vulkan Swap Chain.\nVulkan "
                          "Error Code: [{}]",
                          result));
  }

  vkGetSwapchainImagesKHR(vulkanLogicalDevice, vulkanSwapChain, &imageCount,
                          nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(vulkanLogicalDevice, vulkanSwapChain, &imageCount,
                          swapChainImages.data());
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void Vulkan::CreateImageViews() {
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];

    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vulkanLogicalDevice, &createInfo, nullptr,
                          &swapChainImageViews[i]) != VK_SUCCESS) {
      ShowError("Vulkan Image View", "Unable to create Vulkan Image View");
    }
  }
}

void Vulkan::CreateGraphicsPipeline() {}
