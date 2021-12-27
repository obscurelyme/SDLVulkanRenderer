#ifndef _coffeemaker_vulkan_commands_hpp
#define _coffeemaker_vulkan_commands_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <cmath>
#include <vector>

#include "SimpleMessageBox.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"

class VulkanCommands {
  public:
  VulkanCommands() :
      _logicalDevice(nullptr),
      _physicalDevice(nullptr),
      _swapChain(nullptr),
      _renderPass(nullptr),
      _framebuffer(nullptr),
      _commandPool(VK_NULL_HANDLE),
      _commandBuffers({}),
      _mainCommandBuffer(VK_NULL_HANDLE),
      clearColor({}) {
    // NOTE: default clear color is black
    clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
  }

  ~VulkanCommands() {
    if (_logicalDevice != nullptr && _logicalDevice->Handle != nullptr) {
      DestroyPool();
    }
  }

  void DestroyPool() {
    if (_commandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(_logicalDevice->Handle, _commandPool, nullptr);
      _commandPool = VK_NULL_HANDLE;
    }
  }

  void FreeCommandBuffers() {
    if (_logicalDevice != nullptr && _logicalDevice->Handle != VK_NULL_HANDLE) {
      vkFreeCommandBuffers(_logicalDevice->Handle, _commandPool, static_cast<uint32_t>(1), &_mainCommandBuffer);

      // vkFreeCommandBuffers(_logicalDevice->Handle, _commandPool, static_cast<uint32_t>(_commandBuffers.size()),
      //                      _commandBuffers.data());
    }
  }

  void SetPhysicalDevice(VulkanPhysicalDevice* device) { _physicalDevice = device; }

  void SetLogicalDevice(VulkanLogicalDevice* device) { _logicalDevice = device; }

  void SetSwapchain(VulkanSwapchain* swapchain) { _swapChain = swapchain; }

  void SetRenderPass(VulkanRenderPass* renderPass) { _renderPass = renderPass; }

  void SetFramebuffers(VulkanFramebuffer* framebuffer) { _framebuffer = framebuffer; }

  void CreateCommandPool() {
    VulkanQueueFamilyIndices queueFamilyIndices = _physicalDevice->QueueFamilies;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(_logicalDevice->Handle, &poolInfo, nullptr, &_commandPool);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Command Pool",
                                  fmt::format("Unable to create command pool.\nVulkan Error Code: [{}]", result));
    }
  }

  void CreateCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(1);

    VkResult result = vkAllocateCommandBuffers(_logicalDevice->Handle, &allocInfo, &_mainCommandBuffer);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Command Buffers",
                                  fmt::format("Unable to create command buffers.\nVulkan Error Code: [{}]", result));
    }
  }

  void ResetCommandBuffers() { vkResetCommandBuffer(_mainCommandBuffer, 0); }

  void BeginRecording(int swapchainImageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    vkBeginCommandBuffer(_mainCommandBuffer, &beginInfo);

    // Start the render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass->Handle;
    renderPassInfo.framebuffer = _framebuffer->FramebufferHandles[swapchainImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChain->GetExtent();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(_mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  }

  void SetRenderClearColor(VkClearColorValue color) { clearColor.color = color; }

  void Prerecord() {
    BeginRecording(0);
    // vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdDraw(_mainCommandBuffer, 3, 1, 0, 0);
    EndRecording();
  }

  void EndRecording() {
    vkCmdEndRenderPass(_mainCommandBuffer);
    vkEndCommandBuffer(_mainCommandBuffer);
  }

  VkCommandBuffer& GetBuffer() { return _mainCommandBuffer; }

  private:
  VulkanLogicalDevice* _logicalDevice;
  VulkanPhysicalDevice* _physicalDevice;
  VulkanSwapchain* _swapChain;
  VulkanRenderPass* _renderPass;
  VulkanFramebuffer* _framebuffer;
  VkCommandPool _commandPool;
  std::vector<VkCommandBuffer> _commandBuffers;

  VkCommandBuffer _mainCommandBuffer;
  VkClearValue clearColor;
};

#endif