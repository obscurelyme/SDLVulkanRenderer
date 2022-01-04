#ifndef _coffeemaker_renderer_vulkan_commands_hpp
#define _coffeemaker_renderer_vulkan_commands_hpp

#include <vulkan/vulkan.h>

#include <array>
#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);

  VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level);

  VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags);

  class Commands {
    public:
    static void SetClearValues();
    static void CreateCommandPool();
    static void CreateCommandBuffers();
    static void ResetCommandBuffers(size_t swapChainImageIndex);
    static void BeginRecording(size_t swapchainImageIndex);
    static void EndRecording(size_t swapchainImageIndex);
    static VkCommandBuffer GetCurrentBuffer();
    static void DestroyCommandPool();
    static void FreeCommandBuffers();

    static VkCommandPool commandPool;
    static VkClearValue clearColor;
    static VkClearValue depthClear;
    static std::array<VkClearValue, 2> clearValues;
    static std::vector<VkCommandBuffer> CommandBuffers;
    static size_t CurrentCmdBufferIndex;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
