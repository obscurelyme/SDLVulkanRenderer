#ifndef _coffeemaker_vk_initializers_hpp
#define _coffeemaker_vk_initializers_hpp

#include <vulkan/vulkan.h>

/**
 * Wrapper functions to create CreateInfo objects for Vulkan
 */
namespace VkInit {
  VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0 /*= 0*/);

  VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
      VkCommandPool pool, uint32_t count = 1 /*= 1*/,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/);

  VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0 /*= 0*/);

  VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0 /*= 0*/);

  VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0 /*= 0*/);

  VkSubmitInfo SubmitInfo(VkCommandBuffer* cmd);

  VkPresentInfoKHR PresentInfo();

}  // namespace VkInit

#endif