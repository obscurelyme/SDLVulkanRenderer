
#include "VkUtils.hpp"

#include <SDL2/SDL.h>
#include <fmt/core.h>

#include "VulkanAllocator.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::~Texture() { vmaDestroyBuffer(VulkanAllocator::allocator, buffer.buffer, buffer.allocation); }

AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
  // allocate vertex buffer
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;

  bufferInfo.size = allocSize;
  bufferInfo.usage = usage;

  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;

  AllocatedBuffer newBuffer;

  // allocate the buffer
  vmaCreateBuffer(VulkanAllocator::allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
                  nullptr);

  return newBuffer;
}

void DestroyBuffer(AllocatedBuffer allocBuffer) {
  vmaDestroyBuffer(VulkanAllocator::allocator, allocBuffer.buffer, allocBuffer.allocation);
}

void MapMemory(const void* pData, size_t size, VmaAllocation allocation) {
  void* data;
  vmaMapMemory(VulkanAllocator::allocator, allocation, &data);
  memcpy(data, pData, size);
  vmaUnmapMemory(VulkanAllocator::allocator, allocation);
}

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

Texture StbLoadImage(const std::string& filename) {
  std::string fullfilename = fmt::format("{}{}", SDL_GetBasePath(), filename);
  int width, height, channels;
  stbi_uc* pixels = stbi_load(fullfilename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  Texture t{};
  t.width = width;
  t.height = height;
  t.channels = channels;
  t.filename = filename;
  t.format = VK_FORMAT_R8G8B8A8_SRGB;
  t.size = static_cast<VkDeviceSize>(width * height * 4);

  t.buffer = CreateBuffer(t.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
  MapMemory(pixels, t.size, t.buffer.allocation);
  stbi_image_free(pixels);

  return t;
}
