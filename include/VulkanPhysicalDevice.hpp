#ifndef _coffeemaker_vulkan_physical_device_hpp
#define _coffeemaker_vulkan_physical_device_hpp

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "SimpleMessageBox.hpp"

struct VulkanQueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> transferFamily;

  std::vector<VkQueueFamilyProperties> properties;

  bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }

  bool SupportsTransfer() { return transferFamily.has_value(); }
};

struct VulkanSwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanPhysicalDeviceType {
  static std::string ToString(VkPhysicalDeviceType type) {
    switch (type) {
      case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "Other";
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "Integrated GPU";
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "Discrete GPU";
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "Virtual GPU";
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
      default:
        return "Unknown";
    }
  }
};

class VulkanPhysicalDevice {
  public:
  VulkanPhysicalDevice() :
      Properties({}), Features({}), SupportedExtensions({}), SwapChainSupport({}), QueueFamilies({}) {}

  explicit VulkanPhysicalDevice(VkPhysicalDevice d) :
      Handle(d), Properties({}), Features({}), SupportedExtensions({}), SwapChainSupport({}), QueueFamilies({}) {
    if (d == nullptr) {
      return;
    }
    vkGetPhysicalDeviceProperties(Handle, &Properties);
    vkGetPhysicalDeviceFeatures(Handle, &Features);

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(Handle, nullptr, &extensionCount, nullptr);
    SupportedExtensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(Handle, nullptr, &extensionCount, SupportedExtensions.data());

    vkGetPhysicalDeviceMemoryProperties(Handle, &MemoryProperties);
  }

  VulkanPhysicalDevice(const VulkanPhysicalDevice& d) {
    if (&d == this) {
      return;
    }
    this->Surface = d.Surface;
    this->Handle = d.Handle;
    this->Properties = d.Properties;
    this->Features = d.Features;
    this->SupportedExtensions = d.SupportedExtensions;
    this->SwapChainSupport = d.SwapChainSupport;
    this->QueueFamilies = d.QueueFamilies;
  }

  VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice& rhs) {
    if (&rhs != this) {
      this->Surface = rhs.Surface;
      this->Handle = rhs.Handle;
      this->Properties = rhs.Properties;
      this->Features = rhs.Features;
      this->SupportedExtensions = rhs.SupportedExtensions;
      this->SwapChainSupport = rhs.SwapChainSupport;
      this->QueueFamilies = rhs.QueueFamilies;
    }

    return *this;
  }

  bool operator==(VulkanPhysicalDevice& rhs) { return strcmp(Name(), rhs.Name()) == 0; }

  static std::vector<VulkanPhysicalDevice>& EnumeratePhysicalDevices(VkInstance instance) {
    if (PhysicalDevices.size() > 0) {
      return PhysicalDevices;
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      SimpleMessageBox::ShowError("Vulkan Physical Device", "Unable to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
      VulkanPhysicalDevice newDevice{device};
      PhysicalDevices.push_back(newDevice);
    }

    VulkanPhysicalDevice::RateAllDevices();

    return PhysicalDevices;
  }

  static void ClearAllPhysicalDevices() { PhysicalDevices.clear(); }

  static std::vector<std::string> GetAllPhysicalDeviceInfo() {
    std::vector<std::string> physicalDeviceNames{};

    for (VulkanPhysicalDevice& physicalDevice : PhysicalDevices) {
      physicalDeviceNames.push_back(physicalDevice.ToString());
    }

    return physicalDeviceNames;
  }

  bool AreExtensionsSupported(std::vector<const char*> requestedExtensions) {
    std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    for (const auto& extension : SupportedExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  void FindQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Handle, &queueFamilyCount, nullptr);

    QueueFamilies.properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Handle, &queueFamilyCount, QueueFamilies.properties.data());

    int i = 0;
    for (const auto& queueFamily : QueueFamilies.properties) {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(Handle, i, Surface, &presentSupport);

      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        QueueFamilies.graphicsFamily = i;
      }

      if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
        QueueFamilies.transferFamily = i;
      }

      if (presentSupport) {
        QueueFamilies.presentFamily = i;
      }

      if (QueueFamilies.IsComplete()) {
        break;
      }

      i++;
    }
  }

  void QuerySwapChainSupport() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Handle, Surface, &SwapChainSupport.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Handle, Surface, &formatCount, nullptr);
    if (formatCount != 0) {
      SwapChainSupport.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(Handle, Surface, &formatCount, SwapChainSupport.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Handle, Surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
      SwapChainSupport.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(Handle, Surface, &presentModeCount,
                                                SwapChainSupport.presentModes.data());
    }
  }

  void SetSurface(VkSurfaceKHR surface) { Surface = surface; }

  bool IsDiscreteGPU() { return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; }

  bool IsIntegratedGPU() { return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU; }

  const char* Name() { return Properties.deviceName; }

  std::string ToString() {
    return fmt::format("[{}:{}]", Properties.deviceName, VulkanPhysicalDeviceType::ToString(Properties.deviceType));
  }

  VulkanPhysicalDevice HighestRatedDevice() {
    int bestScore = 0;
    VulkanPhysicalDevice bestDevice;

    for (auto& ratedDevice : RatedPhysicalDevices) {
      if (bestScore < ratedDevice.first) {
        bestDevice = ratedDevice.second;
        bestScore = ratedDevice.first;
      }
    }

    return bestDevice;
  }

  int32_t FindMemoryType(uint32_t requiredMemoryBits, VkMemoryPropertyFlags requiredPropertyFlags) {
    for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++) {
      const bool isRequiredMemoryType = requiredMemoryBits & (1 << i);
      const bool hasRequiredProperties = MemoryProperties.memoryTypes[i].propertyFlags & requiredPropertyFlags;

      if (hasRequiredProperties && isRequiredMemoryType) {
        return static_cast<int32_t>(i);
      }
    }

    return -1;  // No suitable memory type found
  }

  VkSurfaceKHR Surface{VK_NULL_HANDLE};
  VkPhysicalDevice Handle{VK_NULL_HANDLE};
  VkPhysicalDeviceMemoryProperties MemoryProperties;
  VkPhysicalDeviceProperties Properties;
  VkPhysicalDeviceFeatures Features;
  std::vector<VkExtensionProperties> SupportedExtensions;
  VulkanSwapChainSupportDetails SwapChainSupport;
  VulkanQueueFamilyIndices QueueFamilies;

  static std::vector<VulkanPhysicalDevice> PhysicalDevices;
  static std::multimap<int, VulkanPhysicalDevice> RatedPhysicalDevices;

  private:
  static void RateAllDevices() {
    for (auto& device : PhysicalDevices) {
      int score = RateDevice(device);
      RatedPhysicalDevices.insert(std::make_pair(score, device));
    }
  }

  /**
   * Determine a score for a device based on Properties and Features that are supported.
   *
   * @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceLimits.html
   */
  static int RateDevice(const VulkanPhysicalDevice& device) {
    int score = 0;
    // NOTE: Non-negotiable features. If any of these features/properties are not supported. Return a score of 0.
    // This device should not be used for presenting graphics.
    if (device.Features.geometryShader == VK_FALSE) {
      return score;
    }

    if (device.Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }

    score += device.Properties.limits.maxImageDimension2D;

    return score;
  }
};

#endif
