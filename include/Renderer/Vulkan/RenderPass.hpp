#ifndef _coffeemaker_renderer_vulkan_renderpass_hpp
#define _coffeemaker_renderer_vulkan_renderpass_hpp

#include <vulkan/vulkan.h>

#include <array>
#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  class RenderPass {
    public:
    static void Set(VkRenderPass renderpass);
    static void CreateRenderPass();
    static VkRenderPass GetRenderPass();
    static void Destroy();
    static VkRenderPass gVkpRenderPass;
    static RenderPass* gRenderPass;

    void InitCreateSubpassDependency();
    void InitCreateColorAttachmentDes();
    void InitCreateColorAttachmentRef();
    void InitCreateDepthAttachmentDes();
    void InitCreateDepthAttachmentRef();
    void InitCreateSubPassDes();
    void InitCreateRenderPassInfo();
    void InitCreateRenderPass();

    VkRenderPass vkpRenderPass{VK_NULL_HANDLE};
    VkRenderPassCreateInfo renderPassInfo{};
    VkSubpassDescription subpassDescription{};
    VkAttachmentReference colorAttachmentReference{};
    VkAttachmentDescription colorAttachmentDescription{};
    std::vector<VkSubpassDependency> subpassDependencies{};
    VkAttachmentDescription depthAttachmentDescription{};
    VkAttachmentReference depthAttachmentReference{};
    std::array<VkAttachmentDescription, 2> attachments{};
    VkFormat depthFormat{};
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
