#ifndef _coffeemaker_renderer_vulkan_utilities_hpp
#define _coffeemaker_renderer_vulkan_utilities_hpp

/****************************************************************
 * Miscellaneous Create Info helpers ****************************
 ****************************************************************/

#include <vulkan/vulkan.h>

namespace CoffeeMaker::Renderer::Vulkan {

  VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

  VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
