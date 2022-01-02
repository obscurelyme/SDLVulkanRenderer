#include "Renderer/Vulkan/LogicalDevice.hpp"

VkDevice CoffeeMaker::Renderer::Vulkan::LogicalDevice::gLogicalDevice = VK_NULL_HANDLE;

VkDevice CoffeeMaker::Renderer::Vulkan::LogicalDevice::GetLogicalDevice() { return gLogicalDevice; }

void CoffeeMaker::Renderer::Vulkan::LogicalDevice::Set(VkDevice device) { gLogicalDevice = device; }