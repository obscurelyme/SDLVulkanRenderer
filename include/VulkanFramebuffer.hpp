#ifndef _coffeemaker_vulkan_framebuffer_hpp
#define _coffeemaker_vulkan_framebuffer_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include "SimpleMessageBox.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"

class VulkanFramebuffer {
  public:
  VulkanFramebuffer() : _logicalDevice(nullptr), _swapChain(nullptr), _renderPass(nullptr), FramebufferHandles({}) {}

  ~VulkanFramebuffer() {}

  void ClearFramebufferHandles() {
    if (_logicalDevice->Handle != nullptr) {
      for (auto framebuffer : FramebufferHandles) {
        vkDestroyFramebuffer(_logicalDevice->Handle, framebuffer, nullptr);
      }
    }
  }

  void SetLogicalDevice(VulkanLogicalDevice* logicalDevice) { _logicalDevice = logicalDevice; }

  void SetSwapchain(VulkanSwapchain* swapchain) { _swapChain = swapchain; }

  void SetRenderPass(VulkanRenderPass* renderPass) { _renderPass = renderPass; }

  void Build() {
    auto swapChainImageViews = _swapChain->GetImageViews();
    auto swapChainExtent = _swapChain->GetExtent();

    FramebufferHandles.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = _renderPass->Handle;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      VkResult result = vkCreateFramebuffer(_logicalDevice->Handle, &framebufferInfo, nullptr, &FramebufferHandles[i]);

      if (result != VK_SUCCESS) {
        SimpleMessageBox::ShowError(
            "Vulkan Framebuffers",
            fmt::format("Unable to create Vulkan Framebuffer.\nVulkan Error Code: [{}]", result));
      }
    }
  }

  std::vector<VkFramebuffer> FramebufferHandles;

  private:
  VulkanLogicalDevice* _logicalDevice;
  VulkanSwapchain* _swapChain;
  VulkanRenderPass* _renderPass;
};

#endif