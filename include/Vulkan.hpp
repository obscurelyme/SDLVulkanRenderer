#ifndef _coffeemaker_vulkan_hpp
#define _coffeemaker_vulkan_hpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool IsComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan {
public:
  explicit Vulkan(const std::string &applicationName, SDL_Window *window);
  ~Vulkan();

  VkInstance InstanceHandle() const;

  void Draw();

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
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::cerr << "[KHS_Validation_Layer]: " << pCallbackData->pMessage
                << std::endl;
    }

    return VK_FALSE;
  }

private:
  void InitVulkan();
  bool CheckValidationLayerSupport();
  void GetRequiredExtensions();
  void SetupDebugMessenger();
  void SetupValidationLayers();
  VkResult CreateDebugUtilsMessengerEXT(
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator);
  void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks *pAllocator);
  void PopulateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  void PickPhysicalDevice();
  bool IsDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  void CreateLogicalDevice();
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  /**
   * @brief VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application
   * are transferred to the screen right away, which may result in tearing.
   * VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
   * an image from the front of the queue when the display is refreshed and the
   * program inserts rendered images at the back of the queue. If the queue is
   * full then the program has to wait. This is most similar to vertical sync as
   * found in modern games. The moment that the display is refreshed is known as
   * "vertical blank". VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
   * from the previous one if the application is late and the queue was empty at
   * the last vertical blank. Instead of waiting for the next vertical blank,
   * the image is transferred right away when it finally arrives. This may
   * result in visible tearing. VK_PRESENT_MODE_MAILBOX_KHR: This is another
   * variation of the second mode. Instead of blocking the application when the
   * queue is full, the images that are already queued are simply replaced with
   * the newer ones. This mode can be used to render frames as fast as possible
   * while still avoiding tearing, resulting in fewer latency issues than
   * standard vertical sync. This is commonly known as "triple buffering",
   * although the existence of three buffers alone does not necessarily mean
   * that the framerate is unlocked.
   *
   * @param availablePresentModes
   * @return VkPresentModeKHR
   */
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void CreateSwapChain();
  void CreateImageViews();
  void CreateGraphicsPipeline();
  void
  SetupFixedFunctionsPipeline(VkPipelineShaderStageCreateInfo shaderStages[]);
  void CreateRenderPass();
  void CreateFramebuffer();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateSemaphores();

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
  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR vulkanSurface;
  VkPhysicalDevice vulkanPhysicalDevice;
  VkDevice vulkanLogicalDevice;
  VkQueue vulkanGraphicsQueue;
  VkQueue vulkanPresentQueue;
  VkSwapchainKHR vulkanSwapChain;

  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkImageView> swapChainImageViews;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  static int MAX_FRAMES_IN_FLIGHT;
};

#endif
