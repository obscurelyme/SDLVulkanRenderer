#ifndef _coffeemaker_renderer_vulkan_logicaldevice_hpp
#define _coffeemaker_renderer_vulkan_logicaldevice_hpp

#include <vulkan/vulkan.h>

namespace CoffeeMaker::Renderer::Vulkan {

  class LogicalDevice {
    public:
    static void Set(VkDevice device);
    static VkDevice GetLogicalDevice();
    static VkDevice gLogicalDevice;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif