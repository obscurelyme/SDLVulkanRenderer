#ifndef _coffeemaker_renderer_vulkan_material_hpp
#define _coffeemaker_renderer_vulkan_material_hpp

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "Renderer/Vertex.hpp"

namespace CoffeeMaker::Renderer {

  struct Material {
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout layout{VK_NULL_HANDLE};
  };

  struct RenderObject {
    Mesh* mesh;
    Material* material{nullptr};
    glm::mat4 transform{0.0f};
  };

}  // namespace CoffeeMaker::Renderer

#endif
