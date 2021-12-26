#ifndef _coffeemaker_vulkan_physical_device_hpp
#define _coffeemaker_vulkan_physical_device_hpp

#include <vulkan/vulkan.h>
#include <vector>
#include "SimpleMessageBox.hpp"
#include <fmt/core.h>
#include <set>
#include <optional>
#include <iostream>

struct VulkanQueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool IsComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
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
  explicit VulkanPhysicalDevice(VkPhysicalDevice d) : Device(d), Properties({}), Features({}), SupportedExtensions({}) {
      if (d == nullptr) {
        return;
      }
      vkGetPhysicalDeviceProperties(Device, &Properties);
      vkGetPhysicalDeviceFeatures(Device, &Features);

      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, nullptr);
      SupportedExtensions.resize(extensionCount);
      vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, SupportedExtensions.data());

      std::cout << "Device made " << Properties.deviceName << std::endl;
  }

  VulkanPhysicalDevice(const VulkanPhysicalDevice& d) {
    if (&d == this) {
      return;
    }
    this->Device = d.Device;
    this->Properties = d.Properties;
    this->Features = d.Features;
    this->SupportedExtensions = d.SupportedExtensions;
    this->SwapChainSupport = d.SwapChainSupport;
    this->SwapChainSupport.formats = d.SwapChainSupport.formats;
    this->SwapChainSupport.capabilities = d.SwapChainSupport.capabilities;
    this->SwapChainSupport.presentModes = d.SwapChainSupport.presentModes;
    this->QueueFamilies.graphicsFamily = d.QueueFamilies.graphicsFamily;
    this->QueueFamilies.presentFamily = d.QueueFamilies.presentFamily;
  }

  static std::vector<VulkanPhysicalDevice> EnumeratePhysicalDevices(VkInstance instance) {
    if (physicalDevices.size() > 0) {
      return physicalDevices;
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      SimpleMessageBox::ShowError("Vulkan Physical Device",
                "Unable to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


    for (VkPhysicalDevice device : devices) {
      VulkanPhysicalDevice newDevice{device};
      physicalDevices.push_back(newDevice);
    }

    return physicalDevices;
  }

  static std::vector<std::string> GetAllPhysicalDeviceInfo() {
    std::vector<std::string> physicalDeviceNames{};

    for (VulkanPhysicalDevice& physicalDevice : physicalDevices) {
      physicalDeviceNames.push_back(physicalDevice.ToString());
    }

    return physicalDeviceNames;
  }

  bool AreExtensionsSupported(std::vector<const char *> requestedExtensions) {
    std::set<std::string> requiredExtensions(requestedExtensions.begin(),
                                           requestedExtensions.end());

    for (const auto &extension : SupportedExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  void FindQueueFamilies(VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount,
                                            families.data());

    int i = 0;
    for (const auto &queueFamily : families) {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, surface,
                                          &presentSupport);

      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        QueueFamilies.graphicsFamily = i;
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

  void QuerySwapChainSupport(VkSurfaceKHR surface) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, surface,
                                              &SwapChainSupport.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, surface, &formatCount,
                                        nullptr);
    if (formatCount != 0) {
      SwapChainSupport.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(Device, surface, &formatCount,
                                          SwapChainSupport.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, surface,
                                              &presentModeCount, nullptr);
    if (presentModeCount != 0) {
      SwapChainSupport.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          Device, surface, &presentModeCount, SwapChainSupport.presentModes.data());
    }
  }

  bool IsDiscreteGPU() { return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; }

  bool IsIntegratedGPU() { return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU; }

  const char * Name() { return Properties.deviceName; }

  std::string ToString() {
    return fmt::format("[{}:{}]", Properties.deviceName, VulkanPhysicalDeviceType::ToString(Properties.deviceType));
  }

  VkPhysicalDevice Device;
  VkPhysicalDeviceProperties Properties;
  VkPhysicalDeviceFeatures Features;
  std::vector<VkExtensionProperties> SupportedExtensions;
  VulkanSwapChainSupportDetails SwapChainSupport;
  VulkanQueueFamilyIndices QueueFamilies;


  static std::vector<VulkanPhysicalDevice> physicalDevices;
};

#endif
