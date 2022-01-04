#ifndef _coffeemaker_renderer_vulkan_synchronization_hpp
#define _coffeemaker_renderer_vulkan_synchronization_hpp

#include <vulkan/vulkan.h>

#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  class Synchronization {
    public:
    static void CreateSyncTools();
    static void DestroySyncTools();
    static void WaitForFence(size_t currentFrame);
    static void ResetFence(size_t currentFrame);
    // static void WaitForImageAvailable(size_t imageIndex);
    // static void WaitForImageRender(size_t imageIndex);

    static VkSemaphoreCreateInfo semaphoreCreateInfo;
    static VkFenceCreateInfo fenceCreateInfo;

    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> imageRenderSemaphores;
    static std::vector<VkFence> inFlightFences;
    static std::vector<VkFence> imagesInFlight;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
