#ifndef _coffeemaker_vulkanshadermanager_hpp
#define _coffeemaker_vulkanshadermanager_hpp

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanShaderManager {
public:
  /**
   * @brief Reads a compiled shader binary for Vulkan to use later.
   *
   * @param filename
   * @return std::vector<char>
   */
  static std::vector<char> ReadShaderFile(const std::string &filename) {
    std::string fullFilePath = fmt::format("{}{}", SDL_GetBasePath(), filename);
    std::ifstream file{fullFilePath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
      std::cerr << fmt::format("Could not open shader binary {}", filename)
                << std::endl;
      exit(2);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    shaderByteCodes.try_emplace(filename, buffer);

    return buffer;
  }

  static std::vector<char> UseShaderFile(const std::string &filename) {
    auto elem = shaderByteCodes.find(filename);
    if (elem != shaderByteCodes.end()) {
      return elem->second;
    } else {
      std::cerr
          << fmt::format(
                 "Could not use shader binary \"{}\" because it was not "
                 "loaded.\n"
                 "Please ReadShaderFile first before calling UserShaderFile",
                 filename)
          << std::endl;
      exit(3);
    }
  }

  static VkShaderModule CreateShaderModule(VkDevice logicalDevice,
                                           const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS) {
      std::cerr << fmt::format("Failed to create shader module.") << std::endl;
      exit(4);
    }

    return shaderModule;
  }

  static void CleanupShaderModule(VkDevice logicalDevice,
                                  VkShaderModule shaderModule) {
    vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
  }

private:
  static std::map<std::string, std::vector<char>> shaderByteCodes;
};

#endif
