#include "Renderer/Vulkan/MemoryAllocator.hpp"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

VmaAllocator CoffeeMaker::Renderer::Vulkan::MemoryAllocator::gAllocator{VK_NULL_HANDLE};
VmaAllocatorCreateInfo CoffeeMaker::Renderer::Vulkan::MemoryAllocator::gAllocatorCreateInfo{};

void CoffeeMaker::Renderer::Vulkan::MemoryAllocator::CreateAllocator(VkPhysicalDevice physicalDevice,
                                                                     VkDevice logicalDevice, VkInstance instance) {
  gAllocatorCreateInfo.physicalDevice = physicalDevice;
  gAllocatorCreateInfo.device = logicalDevice;
  gAllocatorCreateInfo.instance = instance;

  vmaCreateAllocator(&gAllocatorCreateInfo, &gAllocator);
}

void CoffeeMaker::Renderer::Vulkan::MemoryAllocator::DestroyAllocator() { vmaDestroyAllocator(gAllocator); }

VmaAllocator CoffeeMaker::Renderer::Vulkan::MemoryAllocator::GetAllocator() { return gAllocator; }

CoffeeMaker::Renderer::Vulkan::AllocatedBuffer CoffeeMaker::Renderer::Vulkan::CreateBuffer(size_t allocSize,
                                                                                           VkBufferUsageFlags usage,
                                                                                           VmaMemoryUsage memoryUsage) {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  // allocate vertex buffer
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;

  bufferInfo.size = allocSize;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;

  AllocatedBuffer newBuffer;

  // allocate the buffer
  VkResult r = vmaCreateBuffer(MemAlloc::GetAllocator(), &bufferInfo, &vmaallocInfo, &newBuffer.buffer,
                               &newBuffer.allocation, nullptr);

  if (r != VK_SUCCESS) {
    exit(5555);
  }

  return newBuffer;
}

void CoffeeMaker::Renderer::Vulkan::DestroyBuffer(CoffeeMaker::Renderer::Vulkan::AllocatedBuffer allocBuffer) {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  vmaDestroyBuffer(MemAlloc::GetAllocator(), allocBuffer.buffer, allocBuffer.allocation);
}

void CoffeeMaker::Renderer::Vulkan::MapMemory(const void* pData, size_t size, VmaAllocation allocation) {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  void* data;
  vmaMapMemory(MemAlloc::GetAllocator(), allocation, &data);
  memcpy(data, pData, size);
}

void CoffeeMaker::Renderer::Vulkan::UnmapMemory(VmaAllocation allocation) {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  vmaUnmapMemory(MemAlloc::GetAllocator(), allocation);
}

void CoffeeMaker::Renderer::Vulkan::FlushMemory(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) {
  using MemAlloc = CoffeeMaker::Renderer::Vulkan::MemoryAllocator;

  vmaFlushAllocation(MemAlloc::GetAllocator(), allocation, offset, size);
}
