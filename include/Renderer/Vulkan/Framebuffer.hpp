#ifndef _coffeemaker_renderer_vulkan_framebuffer_hpp
#define _coffeemaker_renderer_vulkan_framebuffer_hpp

#include <vulkan/vulkan.h>

#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  class Framebuffer {
    public:
    static void ClearFramebuffers();
    static void CreateFramebuffers();

    static std::vector<VkFramebuffer> framebuffers;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
