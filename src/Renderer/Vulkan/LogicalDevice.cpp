#include "Renderer/Vulkan/LogicalDevice.hpp"

#include <SDL2/SDL.h>

#include "Renderer/Vulkan/PhysicalDevice.hpp"
#include "Renderer/Vulkan/Utilities.hpp"

VkDevice CoffeeMaker::Renderer::Vulkan::LogicalDevice::gLogicalDevice{VK_NULL_HANDLE};
VkQueue CoffeeMaker::Renderer::Vulkan::LogicalDevice::GraphicsQueue{VK_NULL_HANDLE};
VkQueue CoffeeMaker::Renderer::Vulkan::LogicalDevice::PresentQueue{VK_NULL_HANDLE};
VkQueue CoffeeMaker::Renderer::Vulkan::LogicalDevice::TransferQueue{VK_NULL_HANDLE};
std::vector<const char*> CoffeeMaker::Renderer::Vulkan::LogicalDevice::Extensions{};
std::vector<const char*> CoffeeMaker::Renderer::Vulkan::LogicalDevice::Layers{};
std::vector<VkDeviceQueueCreateInfo> CoffeeMaker::Renderer::Vulkan::LogicalDevice::queueCreateInfos{};
VkDeviceCreateInfo CoffeeMaker::Renderer::Vulkan::LogicalDevice::logicalDeviceCreateInfo{};
bool CoffeeMaker::Renderer::Vulkan::LogicalDevice::validationLayersEnabled{false};

VkDevice CoffeeMaker::Renderer::Vulkan::LogicalDevice::GetLogicalDevice() { return gLogicalDevice; }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::Set(VkDevice device) { gLogicalDevice = device; }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::Destroy() { vkDestroyDevice(gLogicalDevice, nullptr); }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::SetExentions(const std::vector<const char*>& e) { Extensions = e; }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::SetLayers(const std::vector<const char*>& l) { Layers = l; }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::CreateLogicalDevice(bool enableValidationLayers) {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using QueueFamilies = CoffeeMaker::Renderer::Vulkan::VulkanQueueFamilyIndices;
  QueueFamilies families = PhysicalDevice::GetPhysicalDeviceInUse()->QueueFamilies;

  validationLayersEnabled = enableValidationLayers;

  InitCreateQueueInfos();
  InitLogicalDeviceCreateInfo();

  VkResult result =
      vkCreateDevice(PhysicalDevice::GetVkpPhysicalDeviceInUse(), &logicalDeviceCreateInfo, nullptr, &gLogicalDevice);
  if (result != VK_SUCCESS) {
    SDL_LogError(0, "Unable to create Vulkan Logical Device.\nVulkan Error Code: [%d]", result);
    exit(7777);
  }

  vkGetDeviceQueue(gLogicalDevice, families.graphicsFamily.value(), 0, &GraphicsQueue);
  vkGetDeviceQueue(gLogicalDevice, families.presentFamily.value(), 0, &PresentQueue);
  if (families.SupportsTransfer()) {
    vkGetDeviceQueue(gLogicalDevice, families.transferFamily.value(), 0, &TransferQueue);
  }
}

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::InitLogicalDeviceCreateInfo() {
  logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  logicalDeviceCreateInfo.pNext = nullptr;
  logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  logicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  logicalDeviceCreateInfo.pEnabledFeatures = nullptr;  // NOTE: Optional
  logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
  logicalDeviceCreateInfo.ppEnabledExtensionNames = Extensions.data();

  if (IsValidationLayersEnabled()) {
    logicalDeviceCreateInfo.enabledLayerCount = Layers.size();
    logicalDeviceCreateInfo.ppEnabledLayerNames = Layers.data();
  } else {
    logicalDeviceCreateInfo.enabledLayerCount = 0;
    logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr;
  }
}

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::InitCreateQueueInfos() {
  using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;
  using QueueFamilies = CoffeeMaker::Renderer::Vulkan::VulkanQueueFamilyIndices;
  QueueFamilies families = PhysicalDevice::GetPhysicalDeviceInUse()->QueueFamilies;

  std::set<uint32_t> uniqueQueueFamilies = {families.graphicsFamily.value(), families.presentFamily.value()};
  float queuePriority = 1.0f;

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
}

bool CoffeeMaker::Renderer::Vulkan::LogicalDevice::IsValidationLayersEnabled() { return validationLayersEnabled; }
