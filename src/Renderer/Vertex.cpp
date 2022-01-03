#include "Renderer/Vertex.hpp"

#include <vulkan/vulkan.h>

CoffeeMaker::Renderer::Vulkan::VertexInputDescription CoffeeMaker::Renderer::Vertex::Description() {
  CoffeeMaker::Renderer::Vulkan::VertexInputDescription desc;

  VkVertexInputBindingDescription mainBinding{};
  mainBinding.binding = 0;
  mainBinding.stride = sizeof(Vertex);
  mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  desc.bindings.push_back(mainBinding);

  // Position at location 0
  VkVertexInputAttributeDescription positionAttribute{};
  positionAttribute.binding = 0;
  positionAttribute.location = 0;
  positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;  // RGB signed float values. Long way of saying vec3 of floats
  positionAttribute.offset = offsetof(Vertex, position);
  // Normal at location 1
  VkVertexInputAttributeDescription normalAttribute{};
  normalAttribute.binding = 0;
  normalAttribute.location = 1;
  normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;  // RGB signed float values. Long way of saying vec3 of floats
  normalAttribute.offset = offsetof(Vertex, normal);
  // Color at location 2
  VkVertexInputAttributeDescription colorAttribute{};
  colorAttribute.binding = 0;
  colorAttribute.location = 2;
  colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;  // RGB signed float values. Long way of saying vec3 of floats
  colorAttribute.offset = offsetof(Vertex, color);

  // Add all attributes to the input description
  desc.attributes.push_back(positionAttribute);
  desc.attributes.push_back(normalAttribute);
  desc.attributes.push_back(colorAttribute);

  return desc;
}

void CoffeeMaker::Renderer::Mesh::CreateVertexBuffer() {
  using namespace CoffeeMaker::Renderer::Vulkan;

  // Allocate the Vertex Buffer
  vertexBuffer =
      CreateBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  // Map memory to GPU
  MapMemory(vertices.data(), vertices.size() * sizeof(Vertex), vertexBuffer.allocation);
  // Flush allocation as it *may* not be in GPU cache after mapping
  FlushMemory(vertexBuffer.allocation, 0, vertices.size() * sizeof(Vertex));
  UnmapMemory(vertexBuffer.allocation);
}

void CoffeeMaker::Renderer::Mesh::CreateIndexBuffer() {
  using namespace CoffeeMaker::Renderer::Vulkan;

  indexBuffer =
      CreateBuffer(indices.size() * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  MapMemory(indices.data(), indices.size() * sizeof(uint16_t), indexBuffer.allocation);
  FlushMemory(indexBuffer.allocation, 0, indices.size() * sizeof(uint16_t));
  UnmapMemory(indexBuffer.allocation);
}
