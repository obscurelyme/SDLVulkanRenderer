#include "VkInitializers.hpp"

VkCommandPoolCreateInfo VkInit::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;

  return info;
}

VkCommandBufferAllocateInfo VkInit::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count,
                                                              VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.commandPool = pool;
  info.commandBufferCount = count;
  info.level = level;

  return info;
}

VkCommandBufferBeginInfo VkInit::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;
  info.pInheritanceInfo = nullptr;
  info.flags = flags;

  return info;
}

VkFenceCreateInfo VkInit::FenceCreateInfo(VkFenceCreateFlags flags) {
  VkFenceCreateInfo info{};

  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;

  return info;
}

VkSemaphoreCreateInfo VkInit::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags) {
  VkSemaphoreCreateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;

  return info;
}

VkSubmitInfo VkInit::SubmitInfo(VkCommandBuffer* cmd) {
  VkSubmitInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  info.pNext = nullptr;
  info.waitSemaphoreCount = 0;
  info.pWaitSemaphores = nullptr;
  info.pWaitDstStageMask = nullptr;
  info.commandBufferCount = 1;
  info.pCommandBuffers = cmd;
  info.signalSemaphoreCount = 0;
  info.pSignalSemaphores = nullptr;

  return info;
}

VkPresentInfoKHR VkInit::PresentInfo() {
  VkPresentInfoKHR info = {};

  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.pNext = nullptr;
  info.swapchainCount = 0;
  info.pSwapchains = nullptr;
  info.pWaitSemaphores = nullptr;
  info.waitSemaphoreCount = 0;
  info.pImageIndices = nullptr;

  return info;
}