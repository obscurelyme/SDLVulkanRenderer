#ifndef _vk_utils_hpp
#define _vk_utils_hpp

#include <vulkan/vulkan.h>

VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
VkPipelineDepthStencilStateCreateInfo CreateDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                   VkCompareOp compareOp);

#endif