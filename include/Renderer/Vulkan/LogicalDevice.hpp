#ifndef _coffeemaker_renderer_vulkan_logicaldevice_hpp
#define _coffeemaker_renderer_vulkan_logicaldevice_hpp

#include <vulkan/vulkan.h>

#include <vector>

namespace CoffeeMaker::Renderer::Vulkan {

  class LogicalDevice {
    public:
    static void Set(VkDevice device);
    static VkDevice GetLogicalDevice();
    static void Destroy();
    static void CreateLogicalDevice(bool enableValidationLayers);
    static bool IsValidationLayersEnabled();
    static void SetExentions(const std::vector<const char*>& e);
    static void SetLayers(const std::vector<const char*>& l);

    static VkDevice gLogicalDevice;
    static VkQueue GraphicsQueue;
    static VkQueue PresentQueue;
    static VkQueue TransferQueue;
    static std::vector<const char*> Extensions;
    static std::vector<const char*> Layers;
    static std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    static VkDeviceCreateInfo logicalDeviceCreateInfo;

    private:
    static void InitCreateQueueInfos();
    static void InitLogicalDeviceCreateInfo();
    static bool validationLayersEnabled;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
