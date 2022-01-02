#include "Rectangle.hpp"

#include <glm/glm.hpp>

#include "VkUtils.hpp"
#include "VulkanCommands.hpp"
#include "VulkanShaderManager.hpp"

CoffeeMaker::Primitives::Rectangle::Rectangle(VkDevice device, VkRenderPass renderPass, VulkanCommands* commands,
                                              VulkanSwapchain* swapChain) :
    logicalDevice(device), renderPass(renderPass), cmds(commands), swapChain(swapChain) {
  mesh.vertices.resize(4);
  mesh.vertices[0].position = {-0.5f * swapChain->AspectRatioH(), -0.5f, 1.0f};  // bottom left
  mesh.vertices[1].position = {0.5f * swapChain->AspectRatioH(), -0.5f, 1.0f};   // bottom right
  mesh.vertices[2].position = {0.5f * swapChain->AspectRatioH(), 0.5f, 1.0f};    // top right
  mesh.vertices[3].position = {-0.5f * swapChain->AspectRatioH(), 0.5f, 1.0f};   // top left

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
  VkCommandBuffer cmd = cmds->GetCurrentBuffer();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

  glm::mat4 meshMatrix{1.0f};
  MeshPushConstants constants;
  constants.renderMatrix = meshMatrix;
  vkCmdPushConstants(cmd, pipelineBuilder._pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                     &constants);
  vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
  // vkCmdDraw(cmd, mesh.vertices.size(), 0, 0, 0);
}

void CoffeeMaker::Primitives::Rectangle::MakeMeshPipeline() {
  VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(MeshPushConstants)};
  pipeline =
      pipelineBuilder.ShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, VulkanShaderManager::ShaderModule("triangleMesh.spv"))
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