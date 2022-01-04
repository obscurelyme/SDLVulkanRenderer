#include "Renderer/Vulkan/Framebuffer.hpp"

#include <SDL2/SDL.h>

#include <array>

#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/RenderPass.hpp"
#include "Renderer/Vulkan/Swapchain.hpp"

std::vector<VkFramebuffer> CoffeeMaker::Renderer::Vulkan::Framebuffer::framebuffers{};

void CoffeeMaker::Renderer::Vulkan::Framebuffer::CreateFramebuffers() {
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;
  using RenderPass = CoffeeMaker::Renderer::Vulkan::RenderPass;
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  auto swapChain = Swapchain::GetSwapchain();
  auto swapChainImageViews = swapChain->swapChainImageViews;
  auto swapChainExtent = swapChain->extent;

  framebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    std::array<VkImageView, 2> attachments{swapChainImageViews[i], swapChain->depthImageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RenderPass::GetRenderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    VkResult result =
        vkCreateFramebuffer(LogicalDevice::GetLogicalDevice(), &framebufferInfo, nullptr, &framebuffers[i]);

    if (result != VK_SUCCESS) {
      SDL_LogError(0, "Unable to create Vulkan Framebuffer.\nVulkan Error Code: [%d]", result);

      exit(2222);
    }
  }
}

void CoffeeMaker::Renderer::Vulkan::Framebuffer::ClearFramebuffers() {
  using LogicalDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  for (auto framebuffer : framebuffers) {
    vkDestroyFramebuffer(LogicalDevice::GetLogicalDevice(), framebuffer, nullptr);
  }
}
