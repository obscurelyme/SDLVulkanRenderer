#ifndef _coffeemaker_vulkan_commands_hpp
#define _coffeemaker_vulkan_commands_hpp

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "SimpleMessageBox.hpp"
#include "VulkanRenderPass.hpp"
#include <fmt/core.h>

class VulkanCommands {
  public:
  VulkanCommands():
    _logicalDevice(nullptr),
    _physicalDevice(nullptr),
    _swapChain(nullptr),
    _renderPass(nullptr),
    _commandPool(VK_NULL_HANDLE),
    _commandBuffers({}) {}

  ~VulkanCommands() { 
    if (_logicalDevice->Handle != nullptr) {
      vkDestroyCommandPool(_logicalDevice->Handle, _commandPool, nullptr);
    }
  }

  void SetPhysicalDevice(VulkanPhysicalDevice* device) { _physicalDevice = device; }

  void SetLogicalDevice(VulkanLogicalDevice* device) { _logicalDevice = device; }

  void SetSwapchain(VulkanSwapchain* swapchain) { _swapChain = swapchain; }

  void SetRenderPass(VulkanRenderPass* renderPass) { _renderPass = renderPass; }

  void CreateCommandPool() {
    VulkanQueueFamilyIndices queueFamilyIndices = _physicalDevice->QueueFamilies;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(_logicalDevice->Handle, &poolInfo, nullptr,
                                          &_commandPool);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError(
          "Vulkan Command Pool",
          fmt::format("Unable to create command pool.\nVulkan Error Code: [{}]",
                      result));
    }
  }

  void BeginRender(int i) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                                            // Optional
    beginInfo.pInheritanceInfo = nullptr;                           // Optional

    vkBeginCommandBuffer(_commandBuffers[i], &beginInfo);
    

    // Start the render pass
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass->Handle;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChain->GetExtent();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
  }

  void EndRender(int i) {
    vkCmdEndRenderPass(_commandBuffers[i]);
    vkEndCommandBuffer(_commandBuffers[i]);
  }

  private:
  VulkanLogicalDevice* _logicalDevice;
  VulkanPhysicalDevice* _physicalDevice;
  VulkanSwapchain* _swapChain;
  VulkanRenderPass* _renderPass;
  VkCommandPool _commandPool;
  std::vector<VkCommandBuffer> _commandBuffers;
};

#endif