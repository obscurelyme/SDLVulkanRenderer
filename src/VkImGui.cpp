#include "VkImGui.hpp"

VkDescriptorPool VulkanImGui::imguiPool{VK_NULL_HANDLE};
VkDevice VulkanImGui::logicalDevice{VK_NULL_HANDLE};
SDL_Window* VulkanImGui::window{nullptr};
Vulkan* VulkanImGui::renderer{nullptr};