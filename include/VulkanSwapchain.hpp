#ifndef _coffeemaker_vulkan_swapchain_hpp
#define _coffeemaker_vulkan_swapchain_hpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <fmt/core.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "SimpleMessageBox.hpp"
#include "VkUtils.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"

class VulkanSwapchain {
  public:
  VulkanSwapchain() = default;

  ~VulkanSwapchain() {
    if (Handle != VK_NULL_HANDLE && _logicalDevice->Handle != VK_NULL_HANDLE) {
      DestroyHandle();
    }
  }

  void DestroyHandle() {
    for (auto imageView : _swapChainImageViews) {
      vkDestroyImageView(_logicalDevice->Handle, imageView, nullptr);
    }
    // Destroy Depth Image Views and Image
    vkDestroyImageView(_logicalDevice->Handle, _depthImageView, nullptr);
    vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
    // Destroy Swapchain
    vkDestroySwapchainKHR(_logicalDevice->Handle, Handle, nullptr);
    Handle = VK_NULL_HANDLE;
    std::cout << "Swap Chain destroyed" << std::endl;
  }

  void SetWindow(SDL_Window* w) { _window = w; }

  void SetPhysicalDevice(VulkanPhysicalDevice* device) { _physicalDevice = device; }

  void SetLogicalDevice(VulkanLogicalDevice* device) { _logicalDevice = device; }

  void ChooseSwapSurfaceFormat() {
    for (const auto& availableFormat : _physicalDevice->SwapChainSupport.formats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        _surfaceFormat = availableFormat;
        return;
      }
    }

    // NOTE: settle on the first format as it most likely is "good enough"
    _surfaceFormat = _physicalDevice->SwapChainSupport.formats[0];
  }

  void ChoosePresentationMode() {
    for (const auto& availablePresentMode : _physicalDevice->SwapChainSupport.presentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        // Leverage triple buffering if we can.
        _presentMode = availablePresentMode;
        std::cout << "Triple Buffering on" << std::endl;
        return;
      }
    }

    // else, fallback to double buffering
    std::cout << "Double Buffering on" << std::endl;
    _presentMode = VK_PRESENT_MODE_FIFO_KHR;
  }

  void ChooseSwapExtent() {
    int width, height;
    SDL_Vulkan_GetDrawableSize(_window, &width, &height);
    _extent.width = width;
    _extent.height = height;
    VulkanSwapChainSupportDetails details = _physicalDevice->SwapChainSupport;
    _extent = details.capabilities.currentExtent;

    // if (details.capabilities.currentExtent.width != UINT32_MAX) {
    //   _extent = details.capabilities.currentExtent;
    // } else {
    //   int width, height;
    //   SDL_Vulkan_GetDrawableSize(_window, &width, &height);
    //   _extent = {.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height)};

    //   _extent.width = std::clamp(_extent.width, details.capabilities.minImageExtent.width,
    //                              details.capabilities.maxImageExtent.width);
    //   _extent.height = std::clamp(_extent.height, details.capabilities.minImageExtent.height,
    //                               details.capabilities.maxImageExtent.height);
    // }
    std::cout << "SDL Extent: (" << width << ", " << height << ")" << std::endl;
    std::cout << "Physical Device Extent: (" << details.capabilities.currentExtent.width << ", "
              << details.capabilities.currentExtent.height << ")" << std::endl;
  }

  void Create() {
    VulkanSwapChainSupportDetails details = _physicalDevice->SwapChainSupport;
    VulkanQueueFamilyIndices indices = _physicalDevice->QueueFamilies;

    _imageCount = details.capabilities.minImageCount + 1;  // remove here for now...
    if (details.capabilities.maxImageCount > 0 && _imageCount > details.capabilities.maxImageCount) {
      _imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _physicalDevice->Surface;
    createInfo.minImageCount = _imageCount;
    createInfo.imageFormat = _surfaceFormat.format;
    createInfo.imageColorSpace = _surfaceFormat.colorSpace;
    createInfo.imageExtent = _extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::array<uint32_t, 2> queueFamilyIndices{indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;      // Optional
      createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = _presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = VK_SUCCESS;

    if (Handle == VK_NULL_HANDLE) {
      std::cout << "Invoked" << std::endl;
      result = vkCreateSwapchainKHR(_logicalDevice->Handle, &createInfo, nullptr, &Handle);
    } else {
      std::cout << "Already here..." << std::endl;
    }

    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Swap Chain", fmt::format("Unable to create the Vulkan Swap Chain.\nVulkan "
                                                                   "Error Code: [{}]",
                                                                   result));
    }

    vkGetSwapchainImagesKHR(_logicalDevice->Handle, Handle, &_imageCount, nullptr);
    _swapChainImages.resize(_imageCount);
    vkGetSwapchainImagesKHR(_logicalDevice->Handle, Handle, &_imageCount, _swapChainImages.data());
  }

  void CreateImageViews() {
    _swapChainImageViews.resize(_swapChainImages.size());

    for (size_t i = 0; i < _swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = _swapChainImages[i];

      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = _surfaceFormat.format;

      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(_logicalDevice->Handle, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS) {
        SimpleMessageBox::ShowError("Vulkan Image View", "Unable to create Vulkan Image View");
      }
    }
  }

  void CreateDepthImageView(VmaAllocator alloc) {
    _allocator = alloc;
    VkExtent3D depthImageExtent = {.width = _extent.width, .height = _extent.height, .depth = 1};
    VkImageCreateInfo depthInfo =
        CreateImageInfo(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    VmaAllocationCreateInfo dimgAllocInfo = {};
    dimgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image
    vmaCreateImage(alloc, &depthInfo, &dimgAllocInfo, &_depthImage._image, &_depthImage._allocation, nullptr);

    VkImageViewCreateInfo depthViewInfo =
        CreateImageViewInfo(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);

    vkCreateImageView(_logicalDevice->Handle, &depthViewInfo, nullptr, &_depthImageView);
  }

  VkSurfaceFormatKHR GetSurfaceFormat() { return _surfaceFormat; }

  VkExtent2D GetSurfaceExtent() { return _extent; }

  const std::vector<VkImage>& GetImages() { return _swapChainImages; }

  std::vector<VkImageView> GetImageViews() { return _swapChainImageViews; }

  VkPresentModeKHR GetPresentMode() { return _presentMode; }

  uint32_t GetImageCount() { return _imageCount; }

  VkExtent2D GetExtent() { return _extent; }

  VkFormat GetDepthFormat() { return _depthFormat; }

  VkImageView GetDepthImageView() { return _depthImageView; }

  VkSwapchainKHR Handle{VK_NULL_HANDLE};

  private:
  SDL_Window* _window{nullptr};
  VulkanPhysicalDevice* _physicalDevice{nullptr};
  VulkanLogicalDevice* _logicalDevice{nullptr};
  VkSurfaceFormatKHR _surfaceFormat;
  VkPresentModeKHR _presentMode;
  VkExtent2D _extent;
  uint32_t _imageCount;
  std::vector<VkImage> _swapChainImages;
  std::vector<VkImageView> _swapChainImageViews;

  // NOTE: Set up for Depth
  VmaAllocator _allocator;
  VkImageView _depthImageView;
  AllocatedImage _depthImage;
  VkFormat _depthFormat{VK_FORMAT_D32_SFLOAT};
};

#endif