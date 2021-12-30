#ifndef _suzanne_hpp
#define _suzanne_hpp

#include <vulkan/vulkan.h>

// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "Camera.hpp"
#include "KeyboardEvent.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanMesh.hpp"
#include "VulkanShaderManager.hpp"
#include "VulkanSwapchain.hpp"

class Suzanne {
  public:
  Suzanne(VmaAllocator alloc, VkDevice device, VkRenderPass renderPass, VkCommandBuffer command,
          VulkanSwapchain* swapChain);
  ~Suzanne();

  void Draw();

  private:
  void UploadMesh();

  Mesh mesh{};

  VkPipeline _pipeline{VK_NULL_HANDLE};
  PipelineBuilder _pipelineBuilder;

  VmaAllocator allocator{VK_NULL_HANDLE};
  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkCommandBuffer cmd{VK_NULL_HANDLE};
  VulkanSwapchain* swapChain{nullptr};

  bool _orthoMode{false};
  std::shared_ptr<Camera> _mainCamera;
};

#endif