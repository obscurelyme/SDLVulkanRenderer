#include "Renderer/Vulkan/Commands.hpp"

#include <SDL2/SDL.h>

#include "Renderer/Vulkan/Framebuffer.hpp"
#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/PhysicalDevice.hpp"
#include "Renderer/Vulkan/RenderPass.hpp"
#include "Renderer/Vulkan/Swapchain.hpp"

VkCommandPoolCreateInfo CoffeeMaker::Renderer::Vulkan::CommandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                                             VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;

  return info;
}

VkCommandBufferAllocateInfo CoffeeMaker::Renderer::Vulkan::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count,
                                                                                     VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.commandPool = pool;
  info.commandBufferCount = count;
  info.level = level;

  return info;
}

VkCommandBufferBeginInfo CoffeeMaker::Renderer::Vulkan::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info = {};

  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;
  info.pInheritanceInfo = nullptr;
  info.flags = flags;

  return info;
}

VkCommandPool CoffeeMaker::Renderer::Vulkan::Commands::commandPool{VK_NULL_HANDLE};
VkClearValue CoffeeMaker::Renderer::Vulkan::Commands::clearColor{};
VkClearValue CoffeeMaker::Renderer::Vulkan::Commands::depthClear{};
std::array<VkClearValue, 2> CoffeeMaker::Renderer::Vulkan::Commands::clearValues{};
std::vector<VkCommandBuffer> CoffeeMaker::Renderer::Vulkan::Commands::CommandBuffers{};
size_t CoffeeMaker::Renderer::Vulkan::Commands::CurrentCmdBufferIndex{0};

void CoffeeMaker::Renderer::Vulkan::Commands::SetClearValues() {
  clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
  depthClear.depthStencil.depth = 1.0f;

  clearValues[0] = clearColor;
  clearValues[1] = depthClear;
}

void CoffeeMaker::Renderer::Vulkan::Commands::DestroyCommandPool() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  vkDestroyCommandPool(LogicDevice::GetLogicalDevice(), commandPool, nullptr);
  commandPool = VK_NULL_HANDLE;
}

void CoffeeMaker::Renderer::Vulkan::Commands::FreeCommandBuffers() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  vkFreeCommandBuffers(LogicDevice::GetLogicalDevice(), commandPool, static_cast<uint32_t>(CommandBuffers.size()),
                       CommandBuffers.data());
}

void CoffeeMaker::Renderer::Vulkan::Commands::CreateCommandPool() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using VulkanQueueFamilyIndices = CoffeeMaker::Renderer::Vulkan::VulkanQueueFamilyIndices;
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VulkanQueueFamilyIndices queueFamilyIndices = PhysicalDevice::GetPhysicalDeviceInUse()->QueueFamilies;
  uint32_t graphicsFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  VkCommandPoolCreateInfo poolInfo =
      CommandPoolCreateInfo(graphicsFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  VkResult result = vkCreateCommandPool(LogicDevice::GetLogicalDevice(), &poolInfo, nullptr, &commandPool);
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create command pool.\nVulkan Error Code: [%d]", result);
    exit(4444);
  }
}

void CoffeeMaker::Renderer::Vulkan::Commands::CreateCommandBuffers() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
  using Framebuffer = CoffeeMaker::Renderer::Vulkan::Framebuffer;

  CommandBuffers.resize(Framebuffer::framebuffers.size());

  VkCommandBufferAllocateInfo allocInfo =
      CommandBufferAllocateInfo(commandPool, CommandBuffers.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

  VkResult result = vkAllocateCommandBuffers(LogicDevice::GetLogicalDevice(), &allocInfo, CommandBuffers.data());
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create command buffers.\nVulkan Error Code: [%d]", result);
    exit(4444);
  }
}

void CoffeeMaker::Renderer::Vulkan::Commands::ResetCommandBuffers(size_t swapChainImageIndex) {
  vkResetCommandBuffer(CommandBuffers[swapChainImageIndex], 0);
}

void CoffeeMaker::Renderer::Vulkan::Commands::BeginRecording(size_t swapchainImageIndex) {
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;
  using RenderPass = CoffeeMaker::Renderer::Vulkan::RenderPass;
  using Framebuffer = CoffeeMaker::Renderer::Vulkan::Framebuffer;

  VkCommandBufferBeginInfo beginInfo = CommandBufferBeginInfo(0);

  vkBeginCommandBuffer(CommandBuffers[swapchainImageIndex], &beginInfo);

  // Start the render pass
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = RenderPass::GetRenderPass();
  renderPassInfo.framebuffer = Framebuffer::framebuffers[swapchainImageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = Swapchain::GetSwapchain()->extent;
  renderPassInfo.clearValueCount = clearValues.size();
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(CommandBuffers[swapchainImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CoffeeMaker::Renderer::Vulkan::Commands::EndRecording(size_t swapchainImageIndex) {
  vkCmdEndRenderPass(CommandBuffers[swapchainImageIndex]);
  vkEndCommandBuffer(CommandBuffers[swapchainImageIndex]);
}

VkCommandBuffer CoffeeMaker::Renderer::Vulkan::Commands::GetCurrentBuffer() {
  return CommandBuffers[CurrentCmdBufferIndex];
}
