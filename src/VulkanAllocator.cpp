#include "VulkanAllocator.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void VulkanAllocator::CreateAllocator(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkInstance instance) {
  VmaAllocatorCreateInfo info{};
  info.physicalDevice = physicalDevice;
  info.device = logicalDevice;
  info.instance = instance;
  vmaCreateAllocator(&info, &allocator);
}

void VulkanAllocator::DestroyAllocator() { vmaDestroyAllocator(allocator); }

VmaAllocator VulkanAllocator::allocator{VK_NULL_HANDLE};