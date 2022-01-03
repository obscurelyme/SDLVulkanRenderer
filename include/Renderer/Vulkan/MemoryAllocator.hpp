#ifndef _coffeemaker_renderer_vulkan_allocator_hpp
#define _coffeemaker_renderer_vulkan_allocator_hpp

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace CoffeeMaker::Renderer::Vulkan {
  class MemoryAllocator {
    public:
    static void CreateAllocator(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkInstance instance);
    static void DestroyAllocator();
    static VmaAllocator GetAllocator();

    private:
    static VmaAllocator gAllocator;
    static VmaAllocatorCreateInfo gAllocatorCreateInfo;
  };

  struct AllocatedBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
  };

  struct AllocatedImage {
    VkImage image{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
  };

  AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

  void DestroyBuffer(AllocatedBuffer allocBuffer);

  void MapMemory(const void* pData, size_t size, VmaAllocation allocation);

  void UnmapMemory(VmaAllocation allocation);

  void FlushMemory(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size);
}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
