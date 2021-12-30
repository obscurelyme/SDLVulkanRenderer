#ifndef _coffeemaker_vulkan_drawable_hpp
#define _coffeemaker_vulkan_drawable_hpp

#include <vulkan/vulkan.h>

/**
 * TODO: Will work on this later...
 * The idea here is to have a reuable Drawable object that abstracts
 * any common Vulkan functionality
 */
class Drawable {
  public:
  Drawable() = default;
  ~Drawable() = default;

  virtual void Draw() = 0;

  protected:
  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkSwapchainKHR swapChain{VK_NULL_HANDLE};
  VkCommandBuffer cmd{VK_NULL_HANDLE};
};

#endif