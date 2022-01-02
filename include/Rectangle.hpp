#ifndef _coffeemaker_primitive_rectangle_hpp
#define _coffeemaker_primitive_rectangle_hpp

#include <vulkan/vulkan.h>

#include "VulkanCommands.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanMesh.hpp"
#include "VulkanSwapchain.hpp"

namespace CoffeeMaker::Primitives {

  class Rectangle {
    public:
    Rectangle(VkDevice device, VkRenderPass renderPass, VulkanCommands* commands, VulkanSwapchain* swapChain);
    ~Rectangle();

    Mesh mesh{};

    void Draw();

    void MakeMeshPipeline();

    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    PipelineBuilder pipelineBuilder;
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkDevice logicalDevice{VK_NULL_HANDLE};
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VulkanCommands* cmds{nullptr};
    VulkanSwapchain* swapChain{nullptr};
  };

}  // namespace CoffeeMaker::Primitives

#endif
