#ifndef _triangle_hpp
#define _triangle_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <glm/gtx/transform.hpp>
#include <iostream>

#include "Camera.hpp"
#include "DeltaTime.hpp"
#include "Editor/ImGuiEditorObject.hpp"
#include "KeyboardEvent.hpp"
#include "Renderer/Vertex.hpp"
#include "Renderer/Vulkan/Core.hpp"
#include "VulkanCommands.hpp"
#include "VulkanShaderManager.hpp"
#include "VulkanSwapchain.hpp"
#include "imgui.h"

class Triangle : public SDLKeyboardEventListener, public CoffeeMaker::Editor::ImGuiEditorObject {
  using Mesh = CoffeeMaker::Renderer::Mesh;
  using PushConstants = CoffeeMaker::Renderer::MeshPushConstants;
  using Pipeline = CoffeeMaker::Renderer::Vulkan::Pipeline;

  public:
  Triangle(VulkanCommands* commands, VulkanSwapchain* swapChain) :
      cmds(commands), swapChain(swapChain), _mainCamera(Camera::MainCamera()) {
    CreateTriangleMesh();
    MakeMeshPipeline();
  }

  ~Triangle() { DestroyBuffer(mesh.vertexBuffer); }

  void EditorUpdate() override {
    ImGui::Begin("Triangle");
    ImGui::Text("Position: (%f,%f,%f)", 0.0f, 0.0f, 0.0f);
    ImGui::InputFloat("xPos", &position.x, 1.0f, 5.0f);
    ImGui::InputFloat("yPos", &position.y, 1.0f, 5.0f);
    ImGui::InputFloat("zPos", &zIndex, 1.0f, 10.0f);
    ImGui::End();
  }

  void Update() {
    movement = glm::vec2{0.0f};
    if (_moveRight) {
      movement += glm::vec2{1.0f, 0.0f} * speed * CoffeeMaker::DeltaTime::Value();
    }

    if (_moveLeft) {
      movement += glm::vec2{-1.0f, 0.0f} * speed * CoffeeMaker::DeltaTime::Value();
    }

    if (_moveUp) {
      movement += glm::vec2{0.0f, 1.0f} * speed * CoffeeMaker::DeltaTime::Value();
    }

    if (_moveDown) {
      movement += glm::vec2{0.0f, -1.0f} * speed * CoffeeMaker::DeltaTime::Value();
    }

    glm::vec3 no = glm::normalize(glm::vec3(movement, 0.0f));
    position.x += movement.x;
    position.y += movement.y;
  }

  void Draw() {
    VkCommandBuffer cmd = cmds->GetCurrentBuffer();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->GetExtent().width);
    viewport.height = static_cast<float>(swapChain->GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{
                         0,
                         0,
                     },
                     swapChain->GetExtent()};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pPipeline);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

    glm::mat4 model = glm::mat4{1.0f};
    model = glm::translate(model, glm::vec3(position, zIndex));  // vec3 is the position of this object
    // model = glm::rotate(model, glm::radians(_framenumber * 0.4f), glm::vec3{0, 0, 1});
    // model = glm::scale(model, glm::vec3{10, 10, 10});

    /**
     * The camera abstract away the MVP multiplication that is need to create the screen space matrix.
     * The camera object can be toggled to run either orthographic or perspective modes.
     * Press keys to toggle between modes...
     * O = orthographic
     * P = perspective
     */
    glm::mat4 meshMatrix = _mainCamera->ScreenSpaceMatrix(model);
    PushConstants constants;
    constants.renderMatrix = meshMatrix;
    vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &constants);
    vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
  }

  void OnKeyboardEvent(const SDL_KeyboardEvent& event) override {
    if (event.keysym.scancode == SDL_SCANCODE_RIGHT) {
      if (event.type == SDL_KEYDOWN) {
        _moveRight = true;
      } else {
        _moveRight = false;
      }
    }

    if (event.keysym.scancode == SDL_SCANCODE_LEFT) {
      if (event.type == SDL_KEYDOWN) {
        _moveLeft = true;
      } else {
        _moveLeft = false;
      }
    }

    if (event.keysym.scancode == SDL_SCANCODE_UP) {
      if (event.type == SDL_KEYDOWN) {
        _moveUp = true;
      } else {
        _moveUp = false;
      }
    }

    if (event.keysym.scancode == SDL_SCANCODE_DOWN) {
      if (event.type == SDL_KEYDOWN) {
        _moveDown = true;
      } else {
        _moveDown = false;
      }
    }
  }

  void OnSwapChainDestroyed() {
    // noop
    // CleanUpPipelines();
  }

  void OnSwapChainRecreated(VulkanCommands* c, VulkanSwapchain* s) {
    cmds = c;
    swapChain = s;
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
    mesh.vertices[1].color = {0.0f, 1.0f, 0.0f};
    mesh.vertices[2].color = {0.0f, 1.0f, 0.0f};
    // Ignore vertex normals for now
    mesh.CreateVertexBuffer();
  }

  void MakeMeshPipeline() {
    using LogicDevice = CoffeeMaker::Renderer::Vulkan::LogicalDevice;
    using Vertex = CoffeeMaker::Renderer::Vertex;
    using PushConstants = CoffeeMaker::Renderer::MeshPushConstants;
    using PipelineCreateInfo = CoffeeMaker::Renderer::Vulkan::PipelineCreateInfo;

    VkPushConstantRange pushConstants{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(PushConstants)};

    PipelineCreateInfo pipelineCreateInfo{.vertexShader = VulkanShaderManager::ShaderModule("triangleMesh.spv"),
                                          .fragmentShader = VulkanShaderManager::ShaderModule("frag.spv"),
                                          .vertexInputs = Vertex::Description(),
                                          .pushConstantRangeCount = 1,
                                          .pushConstants = pushConstants};

    pipeline.CreatePipeline(pipelineCreateInfo);
  }

  Mesh mesh{};

  Pipeline pipeline;

  VulkanCommands* cmds{nullptr};
  VulkanSwapchain* swapChain{nullptr};
  std::shared_ptr<Camera> _mainCamera;
  glm::vec2 position{0.0f, 0.0f};
  glm::vec2 movement{0.0f};
  float rotation;
  float speed{0.016};

  bool _moveRight{false};
  bool _moveLeft{false};
  bool _moveUp{false};
  bool _moveDown{false};
  float zIndex{0.0f};
};

#endif
