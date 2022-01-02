#ifndef _coffeemaker_renderer_vulkan_renderpass_hpp
#define _coffeemaker_renderer_vulkan_renderpass_hpp

#include <vulkan/vulkan.h>

namespace CoffeeMaker::Renderer::Vulkan {

  class RenderPass {
    public:
    static void Set(VkRenderPass renderpass);
    static VkRenderPass GetRenderPass();
    static VkRenderPass gRenderPass;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif