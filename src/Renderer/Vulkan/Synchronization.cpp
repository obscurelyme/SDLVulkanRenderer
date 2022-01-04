#include "Renderer/Vulkan/Synchronization.hpp"

#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/Swapchain.hpp"

VkSemaphoreCreateInfo CoffeeMaker::Renderer::Vulkan::Synchronization::semaphoreCreateInfo{};
VkFenceCreateInfo CoffeeMaker::Renderer::Vulkan::Synchronization::fenceCreateInfo{};

std::vector<VkSemaphore> CoffeeMaker::Renderer::Vulkan::Synchronization::imageAvailableSemaphores{};
std::vector<VkSemaphore> CoffeeMaker::Renderer::Vulkan::Synchronization::imageRenderSemaphores{};
std::vector<VkFence> CoffeeMaker::Renderer::Vulkan::Synchronization::inFlightFences{};
std::vector<VkFence> CoffeeMaker::Renderer::Vulkan::Synchronization::imagesInFlight{};

void CoffeeMaker::Renderer::Vulkan::Synchronization::CreateSyncTools() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;

  imageAvailableSemaphores.resize(2);
  imageRenderSemaphores.resize(2);
  inFlightFences.resize(2);
  imagesInFlight.resize(Swapchain::GetSwapchain()->swapChainImages.size(), VK_NULL_HANDLE);

  // NOTE: semaphores don't need any flags
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;

  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  /**
   * NOTE: we want to create the fence with the Create Signaled flag,
   * so we can wait on it before using it on a GPU command
   * (for the first frame)
   */
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  fenceCreateInfo.pNext = nullptr;

  for (int i = 0; i < 2; i++) {
    vkCreateSemaphore(LogicDevice::GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
    vkCreateSemaphore(LogicDevice::GetLogicalDevice(), &semaphoreCreateInfo, nullptr, &imageRenderSemaphores[i]);
    vkCreateFence(LogicDevice::GetLogicalDevice(), &fenceCreateInfo, nullptr, &inFlightFences[i]);
  }
}

void CoffeeMaker::Renderer::Vulkan::Synchronization::DestroySyncTools() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  for (int i = 0; i < 2; i++) {
    vkDestroySemaphore(LogicDevice::GetLogicalDevice(), imageRenderSemaphores[i], nullptr);
    vkDestroySemaphore(LogicDevice::GetLogicalDevice(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(LogicDevice::GetLogicalDevice(), inFlightFences[i], nullptr);
  }
}

void CoffeeMaker::Renderer::Vulkan::Synchronization::WaitForFence(size_t currentFrame) {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  vkWaitForFences(LogicDevice::GetLogicalDevice(), 1, &inFlightFences[currentFrame], true, 1000000000);
}

void CoffeeMaker::Renderer::Vulkan::Synchronization::ResetFence(size_t currentFrame) {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  vkResetFences(LogicDevice::GetLogicalDevice(), 1, &inFlightFences[currentFrame]);
}

// void CoffeeMaker::Renderer::Vulkan::Synchronization::WaitForImageAvailable(size_t imageIndex) {
//   using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
// }

// void CoffeeMaker::Renderer::Vulkan::Synchronization::WaitForImageRender(size_t imageIndex) {
//   using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
// }
