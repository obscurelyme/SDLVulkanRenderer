#ifndef _coffeemaker_vulkan_hpp
#define _coffeemaker_vulkan_hpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Editor/ImGuiEditorObject.hpp"
#include "Rectangle.hpp"
#include "Suzanne.hpp"
#include "Triangle.hpp"
#include "VulkanCommands.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanSync.hpp"

struct UploadContext {
  VkFence _uploadFence;
  VkCommandPool _commandPool;
};

class Vulkan : public CoffeeMaker::Editor::ImGuiEditorObject {
  public:
  friend class VulkanShaderManager;

  explicit Vulkan(const std::string &applicationName, SDL_Window *window);
  ~Vulkan();

  VkInstance InstanceHandle() const;
  VkDevice LogicalDevice() const { return logicalDevice.Handle; }

  void Draw();

  void RecreateSwapChain();
  void FramebufferResize();

  void EditorUpdate() override;

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

  void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function);

  void EmitSwapChainWillBeDestroyed();
  void EmitSwapChainCreated();

  static void AddSwapChainDestroyedListener(std::function<void(void)> function);
  static void AddSwapChainCreatedListener(std::function<void(void)> function);

  /**
   * Returns the main renderer for Vulkan.
   */
  static Vulkan *GetRenderer();

  private:
  /**
   * The one and (typically) only instance of Vulkan Renderer
   */
  static Vulkan *_mainRenderer;

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
  void CreateCommands(bool recreation = false);
  void CreateUploadCommands();
  void CreateSemaphores();
  void CreateMemoryAllocator();
  void CreateSurface();
  void InitSyncStructures();

  void ShowError(const std::string &title, const std::string &message);

  bool enableValidationLayers;
  bool vulkanInstanceInitialized;
  SDL_Window *windowHandle;

  public:
  VkInstance vulkanInstance;

  private:
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

  static int MAX_FRAMES_IN_FLIGHT;
  bool framebufferResized;

  public:
  VmaAllocator allocator;
  VulkanPhysicalDevice physicalDevice{nullptr};
  VulkanLogicalDevice logicalDevice;
  VulkanSwapchain swapChain;
  VulkanCommands commands;
  VulkanRenderPass renderPass;
  VulkanFramebuffer framebuffer;
  VulkanSync syncUtils;
  size_t currentFrame{0};
  int framecount;

  Triangle *triangle;
  CoffeeMaker::Primitives::Rectangle *rectangle;
  Suzanne *suzanne;

  // NOTE: use for immediate submit command steps
  UploadContext _uploadContext;

  private:
  std::vector<std::function<void(void)>> swapChainDestroyedListeners{};
  std::vector<std::function<void(void)>> swapChainCreatedListeners{};

  // EDITOR DETAILS
  private:
  // List available physical devices to get info on.
  void Editor_PhysicalDeviceSelection();
  // List available information about the selected device.
  void Editor_PhysicalDeviceInformation();

  size_t selectedPhysicalDeviceIndex{9999};

  bool selectedPresentMode{false};
  std::array<const char *, 55> features{"robustBufferAccess",
                                        "fullDrawIndexUint32",
                                        "imageCubeArray",
                                        "independentBlend",
                                        "geometryShader",
                                        "tessellationShader",
                                        "sampleRateShading",
                                        "dualSrcBlend",
                                        "logicOp",
                                        "multiDrawIndirect",
                                        "drawIndirectFirstInstance",
                                        "depthClamp",
                                        "depthBiasClamp",
                                        "fillModeNonSolid",
                                        "depthBounds",
                                        "wideLines",
                                        "largePoints",
                                        "alphaToOne",
                                        "multiViewport",
                                        "samplerAnisotropy",
                                        "textureCompressionETC2",
                                        "textureCompressionASTC_LDR",
                                        "textureCompressionBC",
                                        "occlusionQueryPrecise",
                                        "pipelineStatisticsQuery",
                                        "vertexPipelineStoresAndAtomics",
                                        "fragmentStoresAndAtomics",
                                        "shaderTessellationAndGeometryPointSize",
                                        "shaderImageGatherExtended",
                                        "shaderStorageImageExtendedFormats",
                                        "shaderStorageImageMultisample",
                                        "shaderStorageImageReadWithoutFormat",
                                        "shaderStorageImageWriteWithoutFormat",
                                        "shaderUniformBufferArrayDynamicIndexing",
                                        "shaderSampledImageArrayDynamicIndexing",
                                        "shaderStorageBufferArrayDynamicIndexing",
                                        "shaderStorageImageArrayDynamicIndexing",
                                        "shaderClipDistance",
                                        "shaderCullDistance",
                                        "shaderFloat64",
                                        "shaderInt64",
                                        "shaderInt16",
                                        "shaderResourceResidency",
                                        "shaderResourceMinLod",
                                        "sparseBinding",
                                        "sparseResidencyBuffer",
                                        "sparseResidencyImage2D",
                                        "sparseResidencyImage3D",
                                        "sparseResidency2Samples",
                                        "sparseResidency4Samples",
                                        "sparseResidency8Samples",
                                        "sparseResidency16Samples",
                                        "sparseResidencyAliased",
                                        "variableMultisampleRate",
                                        "inheritedQueries"};
};

#endif
