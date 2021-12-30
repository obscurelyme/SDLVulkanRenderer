#ifndef _vk_utils_hpp
#define _vk_utils_hpp

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <string>

#include "VulkanMesh.hpp"

struct Texture {
  ~Texture();
  int width;
  int height;
  int channels;
  std::string filename;
  VkDeviceSize size;
  VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
  AllocatedBuffer buffer;
};

VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
VkPipelineDepthStencilStateCreateInfo CreateDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                   VkCompareOp compareOp);
Texture StbLoadImage(const std::string& filename);

AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

void MapMemory(const void* pData, size_t size, VmaAllocation allocation);

#endif