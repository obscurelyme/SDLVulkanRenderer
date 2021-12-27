#ifndef _triangle_hpp
#define _triangle_hpp

#include <vulkan/vulkan.h>

#include "VulkanGraphicsPipeline.hpp"
#include "VulkanShaderManager.hpp"
#include "VulkanSwapchain.hpp"

class Drawable {
  public:
  Drawable() : logicalDevice(VK_NULL_HANDLE), renderPass(VK_NULL_HANDLE) {
    // get the global logical device... ? maybe?
  }

  virtual void Draw() = 0;

  protected:
  VkDevice logicalDevice;
  VkRenderPass renderPass;
};

class Triangle {
  public:
  Triangle(VkDevice device, VkRenderPass renderPass, VkCommandBuffer command, VulkanSwapchain* swapChain) {
    logicalDevice = device;
    cmd = command;
    _pipeline =
        _pipelineBuilder.ShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, VulkanShaderManager::ShaderModule("vert.spv"))
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

  ~Triangle() {
    vkDeviceWaitIdle(logicalDevice);
    vkDestroyPipeline(logicalDevice, _pipeline, nullptr);
  }

  void Draw() {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);
  }

  private:
  PipelineBuilder _pipelineBuilder;
  VkPipeline _pipeline;

  VkDevice logicalDevice;
  VkRenderPass renderPass;
  VkCommandBuffer cmd;
  VulkanSwapchain* swapChain;
};

#endif