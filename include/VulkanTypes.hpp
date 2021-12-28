#ifndef _coffeemaker_vulkan_types_hpp
#define _coffeemaker_vulkan_types_hpp

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

/**
 * AllocatedBuffer is going to hold the buffer we allocate, alongside its allocation data.
 * VkBuffer is a handle to a GPU side Vulkan buffer, and VmaAllocation holds the state
 * that the VMA library uses, like the memory that buffer was allocated from, and
 * its size. We use the VmaAllocation object to manage the
 * buffer allocation itself.
 */
struct AllocatedBuffer {
  VkBuffer buffer;
  VmaAllocation allocation;
};

#endif