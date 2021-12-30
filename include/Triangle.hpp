#ifndef _triangle_hpp
#define _triangle_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <glm/gtx/transform.hpp>
#include <iostream>

#include "Camera.hpp"
#include "KeyboardEvent.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanMesh.hpp"
#include "VulkanShaderManager.hpp"
#include "VulkanSwapchain.hpp"
#include "imgui.h"

class Triangle : public SDLKeyboardEventListener {
  public:
  Triangle(VmaAllocator alloc, VkDevice device, VkRenderPass renderPass, VkCommandBuffer command,
           VulkanSwapchain* swapChain) :
      allocator(alloc),
      logicalDevice(device),
      renderPass(renderPass),
      cmd(command),
      swapChain(swapChain),
      _mainCamera(Camera::MainCamera()) {
    CreateTriangleMesh();
    _pipeline = MakeRGBTrianglePipeline();
    _redTrianglePipeline = MakeRedTrianglePipeline();
    _meshPipeline = MakeMeshPipeline();
    _mainCamera->OnCameraModeChange([&](CameraType t) { _orthoMode = t == CameraType::Orthographic; });
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

  void Update() override {
    ImGui::Begin("Triangle");
    ImGui::Text("Position: (%f,%f,%f)", 0.0f, 0.0f, 0.0f);
    ImGui::InputFloat("xPos", &position.x, 1.0f, 5.0f);
    ImGui::InputFloat("yPos", &position.y, 1.0f, 5.0f);
    ImGui::End();
  }

  void Draw() {
    if (_useRedPipeline) {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _redTrianglePipeline);
      vkCmdDraw(cmd, 3, 1, 0, 0);
    } else {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);
      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

      glm::mat4 model = glm::mat4{1.0f};
      if (_orthoMode) {
        model = glm::translate(model, position);  // vec3 is the position of this object
        // model = glm::rotate(model, glm::radians(_framenumber * 0.4f), glm::vec3{0, 0, 1});
        // model = glm::scale(model, glm::vec3{10, 10, 10});
      } else {
        model = glm::translate(model, position);  // vec3 is the position of this object
        // model = glm::rotate(model, glm::radians(_framenumber * 0.4f), glm::vec3{0, 0, 1});
        // model = glm::scale(model, glm::vec3{.1, .1, .1});
      }

      /**
       * The camera abstract away the MVP multiplication that is need to create the screen space matrix.
       * The camera object can be toggled to run either orthographic or perspective modes.
       * Press keys to toggle between modes...
       * O = orthographic
       * P = perspective
       */
      glm::mat4 meshMatrix = _mainCamera->ScreenSpaceMatrix(model);
      MeshPushConstants constants;
      constants.renderMatrix = meshMatrix;
      vkCmdPushConstants(cmd, _meshPipelineBuilder._pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                         sizeof(MeshPushConstants), &constants);
      vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
    }
    _framenumber++;
  }

  void OnKeyboardEvent(const SDL_KeyboardEvent& event) {
    if (event.keysym.scancode == SDL_SCANCODE_SPACE) {
      if (event.type == SDL_KEYDOWN) {
        _useRedPipeline = true;
      } else {
        _useRedPipeline = false;
      }
    }

    // if (event.keysym.scancode == SDL_SCANCODE_P && event.type == SDL_KEYUP) {
    //   _orthoMode = false;
    // }

    // if (event.keysym.scancode == SDL_SCANCODE_O && event.type == SDL_KEYUP) {
    //   _orthoMode = true;
    // }
    if (event.keysym.scancode == SDL_SCANCODE_RIGHT && event.type == SDL_KEYDOWN) {
      position += glm::vec3{1.0f, 0.0f, 0.0f};
    }

    if (event.keysym.scancode == SDL_SCANCODE_LEFT && event.type == SDL_KEYDOWN) {
      position += glm::vec3{-1.0f, 0.0f, 0.0f};
    }
  }

  private:
  void CreateTriangleMesh() {
    mesh.vertices.resize(3);
    // Set vertex positions
    mesh.vertices[0].position = {0.5f, 0.5f, 0.0f};
    mesh.vertices[1].position = {0.0f, -0.5f, 0.0f};
    mesh.vertices[2].position = {-0.5f, 0.5f, 0.0f};
    // Set all vertices to green color
    mesh.vertices[0].color = {0.0f, 1.0f, 0.0f};
    mesh.vertices[1].color = {1.0f, 0.0f, 0.0f};
    mesh.vertices[2].color = {0.0f, 1.0f, 0.0f};
    // Ignore vertex normals for now
    UploadMesh();
  }

  void UploadMesh() {
    // Allocate the Vertex Buffer
    mesh.vertexBuffer = CreateBuffer(mesh.vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                     VMA_MEMORY_USAGE_CPU_TO_GPU);

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
        .DepthStencil()
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
        .DepthStencil()
        .PipelineLayout(logicalDevice)
        .Build(logicalDevice, renderPass);
  }

  VkPipeline MakeMeshPipeline() {
    VkPushConstantRange pushConstants{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(MeshPushConstants)};
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
        .PipelineLayout(logicalDevice, pushConstants)
        .DepthStencil()
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
  int _framenumber{0};
  bool _orthoMode{false};
  std::shared_ptr<Camera> _mainCamera;
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  float rotation;
};

#endif