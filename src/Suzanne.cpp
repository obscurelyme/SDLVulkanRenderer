#include "Suzanne.hpp"

Suzanne::Suzanne(VmaAllocator alloc, VkDevice device, VkRenderPass renderPass, VkCommandBuffer command,
                 VulkanSwapchain* swapChain) :
    allocator(alloc),
    logicalDevice(device),
    renderPass(renderPass),
    cmd(command),
    swapChain(swapChain),
    _mainCamera(Camera::MainCamera()) {
  mesh.LoadObj("suzanne.obj");
  UploadMesh();

  VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(MeshPushConstants)};
  _pipeline = _pipelineBuilder
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

Suzanne::~Suzanne() {
  vkDeviceWaitIdle(logicalDevice);
  vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
  vkDestroyPipeline(logicalDevice, _pipeline, nullptr);
  vkDestroyPipelineLayout(logicalDevice, _pipelineBuilder._pipelineLayout, nullptr);
}

void Suzanne::Draw() {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);
  glm::mat4 model = glm::mat4{1.0f};
  if (_orthoMode) {
    model = glm::translate(model, glm::vec3{0.0f, 1.0f, 0.0f});  // vec3 is the position of this object
    // model = glm::rotate(model, glm::radians(_framenumber * 0.4f), glm::vec3{0, 0, 1});
    // model = glm::scale(model, glm::vec3{100, 100, 100});
  } else {
    model = glm::translate(model, glm::vec3{0.0f, 1.0f, 0.0f});  // vec3 is the position of this object
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
  vkCmdPushConstants(cmd, _pipelineBuilder._pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                     &constants);
  vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
}

void Suzanne::UploadMesh() {
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