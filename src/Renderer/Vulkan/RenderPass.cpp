#include "Renderer/Vulkan/RenderPass.hpp"

VkRenderPass CoffeeMaker::Renderer::Vulkan::RenderPass::gRenderPass = VK_NULL_HANDLE;

VkRenderPass CoffeeMaker::Renderer::Vulkan::RenderPass::GetRenderPass() { return gRenderPass; }

void CoffeeMaker::Renderer::Vulkan::RenderPass::Set(VkRenderPass renderpass) { gRenderPass = renderpass; }