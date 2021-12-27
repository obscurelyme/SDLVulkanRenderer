#include "VulkanShaderManager.hpp"

VkDevice VulkanShaderManager::logicalDevice = VK_NULL_HANDLE;
std::map<std::string, std::vector<char>> VulkanShaderManager::shaderByteCodes{};
