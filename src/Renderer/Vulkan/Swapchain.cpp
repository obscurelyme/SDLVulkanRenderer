#include "Renderer/Vulkan/Swapchain.hpp"

#include <SDL2/SDL.h>

#include <array>

#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/PhysicalDevice.hpp"
#include "Renderer/Vulkan/Surface.hpp"
#include "Renderer/Vulkan/Utilities.hpp"

CoffeeMaker::Renderer::Vulkan::Swapchain* CoffeeMaker::Renderer::Vulkan::Swapchain::gSwapchain{nullptr};
CoffeeMaker::Renderer::Vulkan::Swapchain* CoffeeMaker::Renderer::Vulkan::Swapchain::gPrevSwapchain{nullptr};

void CoffeeMaker::Renderer::Vulkan::Swapchain::CreateSwapchain() {
  gSwapchain = new Swapchain();
  gSwapchain->InitChooseSwapSurfaceFormat();
  gSwapchain->InitChoosePresentMode();
  gSwapchain->InitChooseSwapExtent();
  gSwapchain->InitCreateImageViews();
  gSwapchain->InitCreateDepthImageView();
}

VkSwapchainKHR CoffeeMaker::Renderer::Vulkan::Swapchain::GetVkpSwapchain() { return gSwapchain->pSwapchain; }

CoffeeMaker::Renderer::Vulkan::Swapchain* CoffeeMaker::Renderer::Vulkan::Swapchain::GetSwapchain() {
  return gSwapchain;
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::Destroy() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  for (auto imageView : gSwapchain->swapChainImageViews) {
    vkDestroyImageView(LogicDevice::GetLogicalDevice(), imageView, nullptr);
  }
  // Destroy Depth Image Views and Image
  vkDestroyImageView(LogicDevice::GetLogicalDevice(), gSwapchain->depthImageView, nullptr);
  vmaDestroyImage(MemAlloc::GetAllocator(), gSwapchain->depthImage.image, gSwapchain->depthImage.allocation);
  // Destroy Swapchain
  vkDestroySwapchainKHR(LogicDevice::GetLogicalDevice(), gSwapchain->pSwapchain, nullptr);
  gSwapchain = VK_NULL_HANDLE;
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitChooseSwapSurfaceFormat() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using SwapchainSupportDetails = CoffeeMaker::Renderer::Vulkan::VulkanSwapChainSupportDetails;

  SwapchainSupportDetails details = PhysicalDevice::GetPhysicalDeviceInUse()->SwapChainSupport;

  for (const auto& availableFormat : details.formats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surfaceFormat = availableFormat;
      return;
    }
  }

  // NOTE: settle on the first format as it most likely is good enough
  surfaceFormat = details.formats[0];
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitChoosePresentMode() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using SwapchainSupportDetails = CoffeeMaker::Renderer::Vulkan::VulkanSwapChainSupportDetails;

  SwapchainSupportDetails details = PhysicalDevice::GetPhysicalDeviceInUse()->SwapChainSupport;

  for (const auto& availablePresentMode : details.presentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = availablePresentMode;
      return;
    }
  }

  presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitChooseSwapExtent() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using SwapchainSupportDetails = CoffeeMaker::Renderer::Vulkan::VulkanSwapChainSupportDetails;

  SwapchainSupportDetails details = PhysicalDevice::GetPhysicalDeviceInUse()->SwapChainSupport;

  extent = details.capabilities.currentExtent;
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitCreateImageViews() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];

    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = surfaceFormat.format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkResult r = vkCreateImageView(LogicDevice::GetLogicalDevice(), &createInfo, nullptr, &swapChainImageViews[i]);

    if (r != VK_SUCCESS) {
      SDL_LogError(0, "Unable to create Vulkan Image View: %d", r);
      exit(6666);
    }
  }
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitCreateDepthImageView() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  VkExtent3D depthImageExtent = {.width = extent.width, .height = extent.height, .depth = 1};
  VkImageCreateInfo depthInfo = CoffeeMaker::Renderer::Vulkan::CreateImageInfo(
      depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

  VmaAllocationCreateInfo dimgAllocInfo = {};
  dimgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  dimgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  VkResult r = vmaCreateImage(MemAlloc::GetAllocator(), &depthInfo, &dimgAllocInfo, &depthImage.image,
                              &depthImage.allocation, nullptr);

  if (r != VK_SUCCESS) {
    SDL_LogError(0, "Vulkan Memory Allocator was unable to create Depth Image View: %d", r);
    exit(6666);
  }

  VkImageViewCreateInfo depthViewInfo =
      CoffeeMaker::Renderer::Vulkan::CreateImageViewInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

  r = vkCreateImageView(LogicDevice::GetLogicalDevice(), &depthViewInfo, nullptr, &depthImageView);

  if (r != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create Vulkan Depth Image View: %d", r);
    exit(6666);
  }
}

void CoffeeMaker::Renderer::Vulkan::Swapchain::InitCreateSwapchainKHR() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using SwapChainSupportDetails = CoffeeMaker::Renderer::Vulkan::VulkanSwapChainSupportDetails;
  using Surface = CoffeeMaker::Renderer::Vulkan::Surface;
  using QueueFamilyIndices = CoffeeMaker::Renderer::Vulkan::VulkanQueueFamilyIndices;

  PhysicalDevice* physicalDeviceInUse = PhysicalDevice::GetPhysicalDeviceInUse();
  SwapChainSupportDetails details = physicalDeviceInUse->SwapChainSupport;
  QueueFamilyIndices queueIndices = physicalDeviceInUse->QueueFamilies;

  imageCount = details.capabilities.minImageCount + 1;
  if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
    imageCount = details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = Surface::GetSurface();
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array<uint32_t, 2> queueFamilyIndices{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
  if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }
  // OldHandle = Handle != VK_NULL_HANDLE ? Handle : VK_NULL_HANDLE;
  createInfo.preTransform = details.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(LogicDevice::GetLogicalDevice(), &createInfo, nullptr, &pSwapchain);

  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create the Vulkan Swap Chain.\nVulkan Error Code: [%d]", result);
    exit(6666);
  }

  result = vkGetSwapchainImagesKHR(LogicDevice::GetLogicalDevice(), pSwapchain, &imageCount, nullptr);
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to acquire the number Vulkan Swap Chain Images.\nVulkan Error Code: [%d]", result);
    exit(6666);
  }
  swapChainImages.resize(imageCount);
  result = vkGetSwapchainImagesKHR(LogicDevice::GetLogicalDevice(), pSwapchain, &imageCount, swapChainImages.data());
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create the Vulkan Swap Chain Images.\nVulkan Error Code: [%d]", result);
    exit(6666);
  }
}
