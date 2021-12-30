#include "VulkanMesh.hpp"

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

VertexInputDescription Vertex::Description() {
  VertexInputDescription desc;

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

void Mesh::LoadObj(const std::string& filename) {
  std::string fullFilePath = fmt::format("{}{}", SDL_GetBasePath(), filename);
  tinyobj::attrib_t vertexAttributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warning;
  std::string error;

  tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warning, &error, fullFilePath.c_str(), nullptr);

  if (!warning.empty()) {
    std::cout << "[TinyObjLoader][WARNING]: " << warning << std::endl;
  }

  if (!error.empty()) {
    std::cout << "[TinyObjLoader][ERROR]: " << error << std::endl;
  }

  // Loop over the shapes
  for (size_t i = 0; i < shapes.size(); i++) {
    size_t indexOffset = 0;
    // Loop over the faces (polygons)
    for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
      int fv = 3;
      // Loop over the vertices in the face
      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[indexOffset + v];

        // vertex position
        tinyobj::real_t vx = vertexAttributes.vertices[3 * idx.vertex_index + 0];
        tinyobj::real_t vy = vertexAttributes.vertices[3 * idx.vertex_index + 1];
        tinyobj::real_t vz = vertexAttributes.vertices[3 * idx.vertex_index + 2];

        // vertex normal
        tinyobj::real_t nx = vertexAttributes.normals[3 * idx.vertex_index + 0];
        tinyobj::real_t ny = vertexAttributes.normals[3 * idx.vertex_index + 1];
        tinyobj::real_t nz = vertexAttributes.normals[3 * idx.vertex_index + 2];

        // copy to vertex
        Vertex newVertex{};
        newVertex.position.x = vx;
        newVertex.position.y = vy;
        newVertex.position.z = vz;

        newVertex.normal.x = nx;
        newVertex.normal.y = ny;
        newVertex.normal.z = nz;

        newVertex.color = newVertex.normal;

        vertices.push_back(newVertex);
      }
      indexOffset += fv;
    }
  }
}