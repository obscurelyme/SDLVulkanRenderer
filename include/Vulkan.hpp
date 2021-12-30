#ifndef _coffeemaker_vulkan_hpp
#define _coffeemaker_vulkan_hpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Suzanne.hpp"
#include "Triangle.hpp"
#include "VulkanCommands.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanSync.hpp"

class Vulkan {
  public:
  friend class VulkanShaderManager;

  explicit Vulkan(const std::string &applicationName, SDL_Window *window);
  ~Vulkan();

  VkInstance InstanceHandle() const;
  VkDevice LogicalDevice() const { return logicalDevice.Handle; }

  void Draw2();

  void RecreateSwapChain();
  void FramebufferResize();

  /**
   * @brief VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like
   * the creation of a resource
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior
   * that is not necessarily an error, but very likely a bug in your application
   *
   * VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that
   * is invalid and may cause crashes
   *
   * @param messageSeverity
   * @param messageType
   * @param pCallbackData
   * @param pUserData
   * @return VKAPI_ATTR
   */
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::cerr << "[KHS_Validation_Layer]: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
  }

  private:
  void CleanupSwapChain();
  void InitVulkan();
  bool CheckValidationLayerSupport();
  void GetRequiredExtensions();
  void SetupDebugMessenger();
  void SetupValidationLayers();
  VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator);
  void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks *pAllocator);
  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  void PickPhysicalDevice();
  bool IsDeviceSuitable(VulkanPhysicalDevice &device);
  void CreateLogicalDevice();
  /**
   * @brief After choosing a physical device, query what extensions are required
   * by Vulkan in order to create a logical device later.
   */
  void AddRequiredDeviceExtensionSupport(VkPhysicalDevice device);
  void CreateSwapChain();
  void CreateRenderPass();
  void CreateFramebuffer();
  void CreateCommands();
  void CreateSemaphores();
  void CreateMemoryAllocator();

  void CreateSurface();

  void ShowError(const std::string &title, const std::string &message);

  bool enableValidationLayers;
  bool vulkanInstanceInitialized;
  SDL_Window *windowHandle;
  VkInstance vulkanInstance;
  VkApplicationInfo vulkanAppInfo;
  VkInstanceCreateInfo vulkanInstanceCreateInfo;

  uint32_t REQUIRED_VULKAN_EXTENSIONS_COUNT;
  std::vector<const char *> REQUIRED_VULKAN_EXTENSIONS{};
  uint32_t VULKAN_LAYERS_COUNT;
  std::vector<const char *> VULKAN_LAYERS{};
  std::vector<const char *> REQUIRED_VULKAN_DEVICE_EXTENSIONS{};
  std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR vulkanSurface;

  // VkPipelineLayout pipelineLayout;
  // VkPipeline graphicsPipeline;

  // VkCommandPool commandPool;
  // std::vector<VkCommandBuffer> commandBuffers;

  // std::vector<VkSemaphore> imageAvailableSemaphores;
  // std::vector<VkSemaphore> renderFinishedSemaphores;
  // std::vector<VkFence> inFlightFences;
  // std::vector<VkFence> imagesInFlight;
  // size_t currentFrame = 0;

  static int MAX_FRAMES_IN_FLIGHT;
  bool framebufferResized;

  VmaAllocator allocator;
  VulkanPhysicalDevice physicalDevice{nullptr};
  VulkanLogicalDevice logicalDevice;
  VulkanSwapchain swapChain;
  VulkanCommands commands;
  VulkanRenderPass renderPass;
  VulkanFramebuffer framebuffer;
  VulkanSync syncUtils;
  int framecount;

  Triangle *triangle;
  Suzanne *suzanne;
};

#endif
