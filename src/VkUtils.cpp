#include "VkUtils.hpp"

VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
  VkImageCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  // NOTE: 1D and 3D images are pretty niche, stick with 2D for most use cases
  info.imageType = VK_IMAGE_TYPE_2D;

  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = 1;
  // NOTE: controls MSAA
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usageFlags;

  return info;
}

VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
  // build a image-view for the depth image to use for rendering
  VkImageViewCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;

  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.image = image;
  info.format = format;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;
  info.subresourceRange.aspectMask = aspectFlags;

  return info;
}

VkPipelineDepthStencilStateCreateInfo CreateDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                   VkCompareOp compareOp) {
  VkPipelineDepthStencilStateCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info.pNext = nullptr;

  info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
  info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
  info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
  info.depthBoundsTestEnable = VK_FALSE;
  info.minDepthBounds = 0.0f;  // Optional
  info.maxDepthBounds = 1.0f;  // Optional
  info.stencilTestEnable = VK_FALSE;

  return info;
}
