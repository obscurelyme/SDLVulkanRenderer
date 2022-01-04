#ifndef _coffeemaker_renderer_vulkan_utilities_hpp
#define _coffeemaker_renderer_vulkan_utilities_hpp

/****************************************************************
 * Miscellaneous Create Info helpers ****************************
 ****************************************************************/

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

  VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

  struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;
    std::vector<VkQueueFamilyProperties> properties;

    bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    bool SupportsTransfer() { return transferFamily.has_value(); }
  };

  struct VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct UploadContext {
    VkFence uploadFence;
    VkCommandPool commandPool;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
