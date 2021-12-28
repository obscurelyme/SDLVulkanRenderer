#ifndef _triangle_hpp
#define _triangle_hpp

#include <vulkan/vulkan.h>

#include <iostream>

#include "KeyboardEvent.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanMesh.hpp"
#include "VulkanShaderManager.hpp"
#include "VulkanSwapchain.hpp"

class Drawable {
  public:
  Drawable() = default;
  ~Drawable() = default;

  virtual void Draw() = 0;

  protected:
  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkRenderPass renderPass{VK_NULL_HANDLE};
};

class Triangle : public SDLKeyboardEventListener {
  public:
  Triangle(VmaAllocator alloc, VkDevice device, VkRenderPass renderPass, VkCommandBuffer command,
           VulkanSwapchain* swapChain) :
      allocator(alloc), logicalDevice(device), renderPass(renderPass), cmd(command), swapChain(swapChain) {
    CreateTriangleMesh();
    _pipeline = MakeRGBTrianglePipeline();
    _redTrianglePipeline = MakeRedTrianglePipeline();
    _meshPipeline = MakeMeshPipeline();
  }

  ~Triangle() {
    vkDeviceWaitIdle(logicalDevice);
    vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    vkDestroyPipeline(logicalDevice, _pipeline, nullptr);
    vkDestroyPipeline(logicalDevice, _redTrianglePipeline, nullptr);
    vkDestroyPipeline(logicalDevice, _meshPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, _pipelineBuilder._pipelineLayout, nullptr);
    vkDestroyPipelineLayout(logicalDevice, _pipelineBuilder2._pipelineLayout, nullptr);
    vkDestroyPipelineLayout(logicalDevice, _meshPipelineBuilder._pipelineLayout, nullptr);
  }

  void Draw() {
    if (_useRedPipeline) {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _redTrianglePipeline);
      vkCmdDraw(cmd, 3, 1, 0, 0);
    } else {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);
      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);
      vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
    }
  }

  void OnKeyboardEvent(const SDL_KeyboardEvent& event) {
    if (event.keysym.scancode == SDL_SCANCODE_SPACE) {
      if (event.type == SDL_KEYDOWN) {
        _useRedPipeline = true;
      } else {
        _useRedPipeline = false;
      }
    }
  }

  private:
  void CreateTriangleMesh() {
    mesh.vertices.resize(3);
    // Set vertex positions
    mesh.vertices[0].position = {1.0f, 1.0f, 0.0f};
    mesh.vertices[1].position = {0.0f, -1.0f, 0.0f};
    mesh.vertices[2].position = {-1.0f, 1.0f, 0.0f};
    // Set all vertices to green color
    mesh.vertices[0].color = {0.0f, 1.0f, 0.0f};
    mesh.vertices[1].color = {0.0f, 1.0f, 0.0f};
    mesh.vertices[2].color = {0.0f, 1.0f, 0.0f};
    // Ignore vertex normals for now
    UploadMesh();
  }

  void UploadMesh() {
    // Allocate the Vertex Buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // Total size, in bytes, of the buffer we wish to allocate
    bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
    // Specifies that this buffer is going to be used as a vertex buffer.
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo, &mesh.vertexBuffer.buffer, &mesh.vertexBuffer.allocation,
                    nullptr);

    void* data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
  }

  VkPipeline MakeRGBTrianglePipeline() {
    return _pipelineBuilder.ShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, VulkanShaderManager::ShaderModule("vert.spv"))
        .ShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, VulkanShaderManager::ShaderModule("frag.spv"))
        .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .Rasterizer(VK_POLYGON_MODE_FILL)
        .MultiSampling()
        .Scissor(0, 0, swapChain->GetSurfaceExtent())
        .Viewport(0.0f, 0.0f, static_cast<float>(swapChain->GetSurfaceExtent().width),
                  static_cast<float>(swapChain->GetSurfaceExtent().height), 0.0f, 1.0f)
        .VertexInputInfo()
        .ColorBlendAttachState()
        .PipelineLayout(logicalDevice)
        .Build(logicalDevice, renderPass);
  }

  VkPipeline MakeRedTrianglePipeline() {
    return _pipelineBuilder2
        .ShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, VulkanShaderManager::ShaderModule("redTriangleVert.spv"))
        .ShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, VulkanShaderManager::ShaderModule("redTriangleFrag.spv"))
        .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .Rasterizer(VK_POLYGON_MODE_FILL)
        .MultiSampling()
        .Scissor(0, 0, swapChain->GetSurfaceExtent())
        .Viewport(0.0f, 0.0f, static_cast<float>(swapChain->GetSurfaceExtent().width),
                  static_cast<float>(swapChain->GetSurfaceExtent().height), 0.0f, 1.0f)
        .VertexInputInfo()
        .ColorBlendAttachState()
        .PipelineLayout(logicalDevice)
        .Build(logicalDevice, renderPass);
  }

  VkPipeline MakeMeshPipeline() {
    return _meshPipelineBuilder
        .ShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, VulkanShaderManager::ShaderModule("triangleMesh.spv"))
        .ShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, VulkanShaderManager::ShaderModule("frag.spv"))
        .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .Rasterizer(VK_POLYGON_MODE_FILL)
        .MultiSampling()
        .Scissor(0, 0, swapChain->GetSurfaceExtent())
        .Viewport(0.0f, 0.0f, static_cast<float>(swapChain->GetSurfaceExtent().width),
                  static_cast<float>(swapChain->GetSurfaceExtent().height), 0.0f, 1.0f)
        .VertexInputInfo(Vertex::Description())
        .ColorBlendAttachState()
        .PipelineLayout(logicalDevice)
        .Build(logicalDevice, renderPass);
  }

  Mesh mesh{};

  PipelineBuilder _pipelineBuilder;
  PipelineBuilder _pipelineBuilder2;
  PipelineBuilder _meshPipelineBuilder;

  VkPipeline _pipeline{VK_NULL_HANDLE};
  VkPipeline _redTrianglePipeline{VK_NULL_HANDLE};
  VkPipeline _meshPipeline{VK_NULL_HANDLE};

  VmaAllocator allocator{VK_NULL_HANDLE};
  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkCommandBuffer cmd{VK_NULL_HANDLE};
  VulkanSwapchain* swapChain{nullptr};
  bool _useRedPipeline{false};
};

#endif