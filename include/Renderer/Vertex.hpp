#ifndef _coffeemaker_renderer_vertex_hpp
#define _coffeemaker_renderer_vertex_hpp

#include <glm/glm.hpp>

#include "Renderer/Vulkan/MemoryAllocator.hpp"
#include "Renderer/Vulkan/Pipeline.hpp"

namespace CoffeeMaker::Renderer {
  struct Vertex {
    using Vec3 = glm::vec3;
    Vec3 position;  // location 0
    Vec3 normal;    // location 1
    Vec3 color;     // location 2

    /**
     * High level abstraction to acquire the VertexInputDescription for
     * this particular Vertex. Each Vertex type will have its own
     * description, which is why this method is static.
     */
    static CoffeeMaker::Renderer::Vulkan::VertexInputDescription Description();
  };

  struct Mesh {
    using AllocatedBuffer = CoffeeMaker::Renderer::Vulkan::AllocatedBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indexBuffer;

    void LoadObj(const std::string& filename);
    void CreateVertexBuffer();
    void CreateIndexBuffer();
  };

  struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 renderMatrix;
  };

}  // namespace CoffeeMaker::Renderer

#endif
