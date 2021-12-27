#ifndef _coffeemaker_vulkan_renderpass_hpp
#define _coffeemaker_vulkan_renderpass_hpp

#include <vulkan/vulkan.h>

#include <vector>

#include "SimpleMessageBox.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanSwapchain.hpp"

class VulkanRenderPass {
  public:
  VulkanRenderPass() :
      _renderPassInfo({}),
      _subpassDescription({}),
      _colorAttachmentDescription({}),
      _colorAttachmentReference({}),
      _subpassDependencies({}) {
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }

  ~VulkanRenderPass() { DestroyHandle(); }

  void DestroyHandle() {
    if (_logicalDevice->Handle != nullptr && Handle != nullptr) {
      vkDestroyRenderPass(_logicalDevice->Handle, Handle, nullptr);
      Handle = nullptr;
    }
  }

  void SetSwapchain(VulkanSwapchain* swapchain) { _swapChain = swapchain; }

  void SetLogicalDevice(VulkanLogicalDevice* logicalDevice) { _logicalDevice = logicalDevice; }

  void Build() {
    CreateSubpassDependency();
    CreateColorAttachmentDes();
    CreateColorAttachmentRef();
    CreateSubPassDes();
    CreateRenderPassInfo();
    CreateRenderPass();
  }

  VkRenderPass Handle{VK_NULL_HANDLE};

  private:
  void CreateSubpassDependency() {
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    _subpassDependencies.push_back(dependency);
  }

  void CreateColorAttachmentDes() {
    _colorAttachmentDescription.format = _swapChain->GetSurfaceFormat().format;
    _colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    _colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    _colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    _colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    _colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }

  void CreateColorAttachmentRef() {
    _colorAttachmentReference.attachment = 0;
    _colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  void CreateSubPassDes() {
    _subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    _subpassDescription.colorAttachmentCount = 1;
    _subpassDescription.pColorAttachments = &_colorAttachmentReference;
  }

  void CreateRenderPassInfo() {
    _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    _renderPassInfo.attachmentCount = 1;
    _renderPassInfo.pAttachments = &_colorAttachmentDescription;
    _renderPassInfo.subpassCount = 1;
    _renderPassInfo.pSubpasses = &_subpassDescription;
    _renderPassInfo.dependencyCount = static_cast<uint32_t>(_subpassDependencies.size());
    _renderPassInfo.pDependencies = _subpassDependencies.data();
  }

  void CreateRenderPass() {
    VkResult result = vkCreateRenderPass(_logicalDevice->Handle, &_renderPassInfo, nullptr, &Handle);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError("Vulkan Render Pass",
                                  fmt::format("Unable to create Vulkan Render Pass.\nVulkan Error Code: [{}]", result));
    }
  }

  VulkanLogicalDevice* _logicalDevice{nullptr};
  VulkanSwapchain* _swapChain{nullptr};

  VkRenderPassCreateInfo _renderPassInfo;
  VkSubpassDescription _subpassDescription;
  VkAttachmentReference _colorAttachmentReference;
  VkAttachmentDescription _colorAttachmentDescription;
  std::vector<VkSubpassDependency> _subpassDependencies;
};

#endif