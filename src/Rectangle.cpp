#include "Rectangle.hpp"

#include <glm/glm.hpp>

#include "VulkanCommands.hpp"
#include "VulkanShaderManager.hpp"

CoffeeMaker::Primitives::Rectangle::Rectangle(VulkanCommands* commands, VulkanSwapchain* swapChain) :
    cmds(commands), swapChain(swapChain), _mainCamera(Camera::MainCamera()) {
  mesh.vertices.resize(4);
  mesh.vertices[0].position = {-0.5f, -0.5f, 1.0f};  // bottom left
  mesh.vertices[1].position = {0.5f, -0.5f, 1.0f};   // bottom right
  mesh.vertices[2].position = {0.5f, 0.5f, 1.0f};    // top right
  mesh.vertices[3].position = {-0.5f, 0.5f, 1.0f};   // top left

  mesh.vertices[0].color = {1.0f, 0.0f, 0.0f};
  mesh.vertices[1].color = {0.0f, 1.0f, 0.0f};
  mesh.vertices[2].color = {0.0f, 0.0f, 1.0f};
  mesh.vertices[3].color = {1.0f, 1.0f, 1.0f};

  mesh.indices = {0, 1, 2, 2, 3, 0};

  mesh.CreateVertexBuffer();
  mesh.CreateIndexBuffer();

  MakeMeshPipeline();
}

CoffeeMaker::Primitives::Rectangle::~Rectangle() {
  DestroyBuffer(mesh.vertexBuffer);
  DestroyBuffer(mesh.indexBuffer);
}

void CoffeeMaker::Primitives::Rectangle::Draw() {
  using PushConstants = CoffeeMaker::Renderer::MeshPushConstants;

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

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBuilder.pPipeline);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

  glm::mat4 meshMatrix{1.0f};
  pushConstants.renderMatrix = _mainCamera->ScreenSpaceMatrix(meshMatrix);
  vkCmdPushConstants(cmd, pipelineBuilder.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);
  vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
}

void CoffeeMaker::Primitives::Rectangle::MakeMeshPipeline() {
  using PushConstants = CoffeeMaker::Renderer::MeshPushConstants;
  using PipelineCreateInfo = CoffeeMaker::Renderer::Vulkan::PipelineCreateInfo;
  using Vertex = CoffeeMaker::Renderer::Vertex;

  VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(PushConstants)};

  PipelineCreateInfo info{.vertexShader = VulkanShaderManager::ShaderModule("triangleMesh.spv"),
                          .fragmentShader = VulkanShaderManager::ShaderModule("frag.spv"),
                          .vertexInputs = Vertex::Description(),
                          .pushConstantRangeCount = 1,
                          .pushConstants = pushConstants};

  pipelineBuilder.CreatePipeline(info);
}
