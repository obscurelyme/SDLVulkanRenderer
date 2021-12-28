#ifndef _coffeemaker_vulkan_mesh_hpp
#define _coffeemaker_vulkan_mesh_hpp

#include <vector>

#include "VulkanTypes.hpp"
#include "glm/vec3.hpp"

struct VertexInputDescription {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkPipelineVertexInputStateCreateFlags flags = 0;
};

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
  static VertexInputDescription Description();
};

struct Mesh {
  std::vector<Vertex> vertices;
  AllocatedBuffer vertexBuffer;
};

#endif