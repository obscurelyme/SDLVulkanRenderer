#ifndef _triangle_hpp
#define _triangle_hpp

#include <vulkan/vulkan.h>

#include <iostream>

#include "KeyboardEvent.hpp"
#include "VulkanGraphicsPipeline.hpp"
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
  Triangle(VkDevice device, VkRenderPass renderPass, VkCommandBuffer command, VulkanSwapchain* swapChain) :
      logicalDevice(device), renderPass(renderPass), cmd(command), swapChain(swapChain) {
    _pipeline = MakeRGBTrianglePipeline();
    _redTrianglePipeline = MakeRedTrianglePipeline();
  }

  ~Triangle() {
    vkDeviceWaitIdle(logicalDevice);
    vkDestroyPipeline(logicalDevice, _pipeline, nullptr);
    vkDestroyPipeline(logicalDevice, _redTrianglePipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, _pipelineBuilder._pipelineLayout, nullptr);
    vkDestroyPipelineLayout(logicalDevice, _pipelineBuilder2._pipelineLayout, nullptr);
  }

  void Draw() {
    if (_useRedPipeline) {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _redTrianglePipeline);
    } else {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    }

    vkCmdDraw(cmd, 3, 1, 0, 0);
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

  PipelineBuilder _pipelineBuilder;
  PipelineBuilder _pipelineBuilder2;
  VkPipeline _pipeline{VK_NULL_HANDLE};
  VkPipeline _redTrianglePipeline{VK_NULL_HANDLE};

  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkCommandBuffer cmd{VK_NULL_HANDLE};
  VulkanSwapchain* swapChain{nullptr};
  bool _useRedPipeline{false};
};

#endif