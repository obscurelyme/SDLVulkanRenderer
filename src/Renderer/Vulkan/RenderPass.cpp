#include "Renderer/Vulkan/RenderPass.hpp"

#include <SDL2/SDL.h>

#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/Swapchain.hpp"

VkRenderPass CoffeeMaker::Renderer::Vulkan::RenderPass::gVkpRenderPass{VK_NULL_HANDLE};
CoffeeMaker::Renderer::Vulkan::RenderPass* CoffeeMaker::Renderer::Vulkan::RenderPass::gRenderPass{nullptr};

VkRenderPass CoffeeMaker::Renderer::Vulkan::RenderPass::GetRenderPass() { return gRenderPass->vkpRenderPass; }

void CoffeeMaker::Renderer::Vulkan::RenderPass::Set(VkRenderPass renderpass) { gVkpRenderPass = renderpass; }

void CoffeeMaker::Renderer::Vulkan::RenderPass::CreateRenderPass() {
  gRenderPass = new RenderPass();
  gRenderPass->InitCreateSubpassDependency();
  gRenderPass->InitCreateColorAttachmentDes();
  gRenderPass->InitCreateDepthAttachmentDes();
  gRenderPass->InitCreateColorAttachmentRef();
  gRenderPass->InitCreateDepthAttachmentRef();
  gRenderPass->InitCreateSubPassDes();
  gRenderPass->InitCreateRenderPassInfo();
  gRenderPass->InitCreateRenderPass();
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::Destroy() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  vkDestroyRenderPass(LogicalDevice::GetLogicalDevice(), gVkpRenderPass, nullptr);
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateSubpassDependency() {
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  subpassDependencies.push_back(dependency);
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateColorAttachmentDes() {
  using Swapchain = CoffeeMaker::Renderer::Vulkan::Swapchain;

  colorAttachmentDescription.format = Swapchain::GetSwapchain()->surfaceFormat.format;
  colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

  colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateDepthAttachmentDes() {
  depthAttachmentDescription.flags = 0;
  depthAttachmentDescription.format = depthFormat;
  depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateColorAttachmentRef() {
  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateDepthAttachmentRef() {
  depthAttachmentReference.attachment = 1;
  depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateSubPassDes() {
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorAttachmentReference;
  subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateRenderPassInfo() {
  attachments[0] = colorAttachmentDescription;
  attachments[1] = depthAttachmentDescription;

  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpassDescription;
  renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
  renderPassInfo.pDependencies = subpassDependencies.data();
}

void CoffeeMaker::Renderer::Vulkan::RenderPass::InitCreateRenderPass() {
  using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;

  VkResult result = vkCreateRenderPass(LogicDevice::GetLogicalDevice(), &renderPassInfo, nullptr, &vkpRenderPass);
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create Vulkan Render Pass.\nVulkan Error Code: [%d]", result);

    exit(8888);
  }
}
