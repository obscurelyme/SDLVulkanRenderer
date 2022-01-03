#ifndef _coffeemaker_renderer_vulkan_physicaldevice_hpp
#define _coffeemaker_renderer_vulkan_physicaldevice_hpp

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Renderer/Vulkan/Utilities.hpp"

namespace CoffeeMaker::Renderer::Vulkan {

  class PhysicalDevice {
    public:
    static std::vector<PhysicalDevice*>& EnumeratePhysicalDevices(VkInstance instance);
    static void ClearAllPhysicalDevices();
    static VkPhysicalDevice GetVkpPhysicalDeviceInUse();
    static PhysicalDevice* GetPhysicalDeviceInUse();
    static void SelectPhysicalDevice(size_t idToSelect);

    explicit PhysicalDevice(VkPhysicalDevice device);
    ~PhysicalDevice() = default;
    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;
    bool operator==(PhysicalDevice& rhs);
    void FindQueueFamilies();
    void QuerySwapchainSupport();
    bool AreExtensionsSupported(std::vector<const char*>& requestedExtensions);

    const char* Name();

    bool IsDiscreteGPU();
    bool IsIntegratedGPU();
    bool IsCPU();
    bool IsVirtualGPU();
    bool IsOtherType();
    bool IsSelected();

    size_t id{0};
    bool selected{false};
    VkSurfaceKHR Surface{VK_NULL_HANDLE};
    VkPhysicalDevice vkpPhysicalDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceMemoryProperties MemoryProperties{};
    VkPhysicalDeviceProperties Properties{};
    VkPhysicalDeviceFeatures Features{};
    std::vector<VkExtensionProperties> SupportedExtensions{};
    VulkanSwapChainSupportDetails SwapChainSupport{};
    VulkanQueueFamilyIndices QueueFamilies{};

    static std::vector<PhysicalDevice*> gPhysicalDevices;
    static std::multimap<int, PhysicalDevice*> gRatedPhysicalDevices;
    static PhysicalDevice* gPhysicalDeviceInUse;
    static size_t _uid;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
