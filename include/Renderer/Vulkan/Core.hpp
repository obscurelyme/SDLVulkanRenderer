#ifndef _coffeemaker_renderer_vulkan_core_hpp
#define _coffeemaker_renderer_vulkan_core_hpp

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "Renderer/Vulkan/Commands.hpp"
#include "Renderer/Vulkan/Framebuffer.hpp"
#include "Renderer/Vulkan/LogicalDevice.hpp"
#include "Renderer/Vulkan/MemoryAllocator.hpp"
#include "Renderer/Vulkan/PhysicalDevice.hpp"
#include "Renderer/Vulkan/Pipeline.hpp"
#include "Renderer/Vulkan/RenderPass.hpp"
#include "Renderer/Vulkan/Surface.hpp"
#include "Renderer/Vulkan/Swapchain.hpp"
#include "Renderer/Vulkan/Synchronization.hpp"
#include "Renderer/Vulkan/Utilities.hpp"

namespace CoffeeMaker::Renderer::Vulkan {
  // class VulkanRenderer {
  //   public:
  //   VulkanRenderer(const std::string &applicationName, SDL_Window *window);
  //   ~VulkanRenderer();

  //   VkInstance VulkanInstance();
  //   void CleanSwapchain();
  //   void RecreateSwapchain();
  //   void Render();
  //   void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function);

  //   static VulkanRenderer *GetRenderer();
  //   static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  //                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
  //                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  //                                                       void *pUserData) {
  //     if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
  //       std::cerr << "[KHS_Validation_Layer]: " << pCallbackData->pMessage << std::endl;
  //     }

  //     return VK_FALSE;
  //   }

  //   VkApplicationInfo vulkanAppInfo;
  //   VkInstanceCreateInfo vulkanInstanceCreateInfo;
  //   uint32_t REQUIRED_VULKAN_EXTENSIONS_COUNT;
  //   std::vector<const char *> REQUIRED_VULKAN_EXTENSIONS{};
  //   uint32_t VULKAN_LAYERS_COUNT;
  //   std::vector<const char *> VULKAN_LAYERS{};
  //   std::vector<const char *> REQUIRED_VULKAN_DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  //   std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  //   VkDebugUtilsMessengerEXT debugMessenger;
  //   VkInstance vulkanInstance;

  //   private:
  //   static VulkanRenderer *renderInstance;
  //   void Init();

  //   size_t currentFrame{0};
  //   UploadContext uploadContext{};
  // };
}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
