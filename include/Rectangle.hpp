#ifndef _coffeemaker_primitive_rectangle_hpp
#define _coffeemaker_primitive_rectangle_hpp

#include <vulkan/vulkan.h>

#include "Camera.hpp"
#include "Renderer/Vertex.hpp"
#include "Renderer/Vulkan/Pipeline.hpp"

namespace CoffeeMaker::Primitives {

  class Rectangle {
    using Mesh = CoffeeMaker::Renderer::Mesh;
    using PushConstants = CoffeeMaker::Renderer::MeshPushConstants;
    using Pipeline = CoffeeMaker::Renderer::Vulkan::Pipeline;

    public:
    Rectangle();
    ~Rectangle();

    Mesh mesh{};
    PushConstants pushConstants{};

    void Draw();

    void MakeMeshPipeline();

    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    Pipeline pipelineBuilder;
    std::shared_ptr<Camera> _mainCamera;
  };

}  // namespace CoffeeMaker::Primitives

#endif
