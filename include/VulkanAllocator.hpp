#ifndef _coffeemaker_vulkan_allocator_hpp
#define _coffeemaker_vulkan_allocator_hpp

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class VulkanAllocator {
  public:
  static void CreateAllocator(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkInstance instance);
  static void DestroyAllocator();
  static VmaAllocator allocator;
};

#endif