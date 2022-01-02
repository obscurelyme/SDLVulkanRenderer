#ifndef _coffeemaker_primitive_rectangle_hpp
#define _coffeemaker_primitive_rectangle_hpp

#include <vulkan/vulkan.h>

#include "Camera.hpp"
#include "Renderer/Vulkan/Pipeline.hpp"
#include "VulkanCommands.hpp"
#include "VulkanMesh.hpp"
#include "VulkanSwapchain.hpp"

namespace CoffeeMaker::Primitives {

  class Rectangle {
    using Pipeline = CoffeeMaker::Renderer::Vulkan::Pipeline;

    public:
    Rectangle(VulkanCommands* commands, VulkanSwapchain* swapChain);
    ~Rectangle();

    Mesh mesh{};

    void Draw();

    void MakeMeshPipeline();

    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    Pipeline pipelineBuilder;
    VulkanCommands* cmds{nullptr};
    VulkanSwapchain* swapChain{nullptr};
    std::shared_ptr<Camera> _mainCamera;
  };

}  // namespace CoffeeMaker::Primitives

#endif
