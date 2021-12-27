#ifndef _coffeemaker_vulkan_sync_hpp
#define _coffeemaker_vulkan_sync_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include "SimpleMessageBox.hpp"

class VulkanSync {
  public:
  VulkanSync() :
      _logicalDevice(nullptr),
      _semaphoreInfo({}),
      _fenceInfo({}),
      _presentSemaphore(VK_NULL_HANDLE),
      _renderSemaphore(VK_NULL_HANDLE),
      _renderFence(VK_NULL_HANDLE) {}

  ~VulkanSync() {
    if (_logicalDevice != nullptr && _logicalDevice->Handle != nullptr) {
      DestroyHandles();
    }
  };

  void DestroyHandles() {
    if (_presentSemaphore != VK_NULL_HANDLE && _renderSemaphore != VK_NULL_HANDLE && _renderFence != VK_NULL_HANDLE) {
      vkDestroySemaphore(_logicalDevice->Handle, _renderSemaphore, nullptr);
      vkDestroySemaphore(_logicalDevice->Handle, _presentSemaphore, nullptr);
      vkDestroyFence(_logicalDevice->Handle, _renderFence, nullptr);
      _presentSemaphore = VK_NULL_HANDLE;
      _renderSemaphore = VK_NULL_HANDLE;
      _renderFence = VK_NULL_HANDLE;
    }
  }

  void SetLogicalDevice(VulkanLogicalDevice* logicalDevice) { _logicalDevice = logicalDevice; }

  void CreateRenderFence() {
    _fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    /**
     * NOTE: we want to create the fence with the Create Signaled flag,
     * so we can wait on it before using it on a GPU command
     * (for the first frame)
     */
    _fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    _fenceInfo.pNext = nullptr;
  }

  void CreateSemaphores() {
    // NOTE: semaphores don't need any flags
    _semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    _semaphoreInfo.pNext = nullptr;
    _semaphoreInfo.flags = 0;
  }

  void Build() {
    VkResult imgAvailResult = vkCreateSemaphore(_logicalDevice->Handle, &_semaphoreInfo, nullptr, &_presentSemaphore);
    VkResult renderFinResult = vkCreateSemaphore(_logicalDevice->Handle, &_semaphoreInfo, nullptr, &_renderSemaphore);

    VkResult fenceResult = vkCreateFence(_logicalDevice->Handle, &_fenceInfo, nullptr, &_renderFence);

    if (imgAvailResult != VK_SUCCESS || renderFinResult != VK_SUCCESS || fenceResult != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Semaphore/Fences",
                                  fmt::format("Creationg of Vulkan Semaphores & Fences failed with the "
                                              "following error codes.\nImage Available Semaphore "
                                              "Result: {}\nRender Finished Semaphore Result: {}\nFence "
                                              "Result: [{}]",
                                              imgAvailResult, renderFinResult, fenceResult));
    }
  }

  void WaitForFence() { vkWaitForFences(_logicalDevice->Handle, 1, &_renderFence, true, 1000000000); }

  void ResetFence() { vkResetFences(_logicalDevice->Handle, 1, &_renderFence); }

  VkSemaphore RenderSemaphore() { return _renderSemaphore; }

  VkSemaphore PresentSemaphore() { return _presentSemaphore; }

  VkFence RenderFence() { return _renderFence; }

  private:
  VulkanLogicalDevice* _logicalDevice;

  VkSemaphoreCreateInfo _semaphoreInfo;
  VkFenceCreateInfo _fenceInfo;

  VkSemaphore _presentSemaphore;
  VkSemaphore _renderSemaphore;
  VkFence _renderFence;
};

#endif