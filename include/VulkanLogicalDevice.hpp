#ifndef _coffeemaker_vulkan_logical_device_hpp
#define _coffeemaker_vulkan_logical_device_hpp

#include <vector>
#include <vulkan/vulkan.h>
#include "VulkanPhysicalDevice.hpp"
#include "SimpleMessageBox.hpp"
#include <set>

class VulkanLogicalDevice {
  public:
  VulkanLogicalDevice():
    Handle(VK_NULL_HANDLE), 
    Extensions({}),
    Layers({}),
    _enableValidationLayers(false),
    _queueCreateInfos({}),
    _logicalDeviceCreateInfo({}) {};

  explicit VulkanLogicalDevice(const VulkanPhysicalDevice& physicalDevice) :
    PhysicalDevice(physicalDevice), 
    Handle(VK_NULL_HANDLE), 
    Extensions({}),
    Layers({}),
    _enableValidationLayers(false),
    _queueCreateInfos({}),
    _logicalDeviceCreateInfo({}) {
      SetUp();
    }
  
  ~VulkanLogicalDevice() {
    if (Handle != VK_NULL_HANDLE) {
      DestroyHandle();
    }
  }

  void DestroyHandle() { 
    vkDestroyDevice(Handle, nullptr);
    Handle = VK_NULL_HANDLE;
  }

  void SetPhysicalDevice(const VulkanPhysicalDevice& physicalDevice) {
    PhysicalDevice = physicalDevice;
  }

  void SetExentions(const std::vector<const char *>& e) {
    Extensions = e;
  }

  void SetLayers(const std::vector<const char *>& l) {
    Layers = l;
  }

  void EnableValidation(bool toggle) { 
    _enableValidationLayers = toggle;
  }

  void Create() {
    _logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    _logicalDeviceCreateInfo.pNext = nullptr;
    _logicalDeviceCreateInfo.pQueueCreateInfos = _queueCreateInfos.data();
    _logicalDeviceCreateInfo.queueCreateInfoCount =
        static_cast<uint32_t>(_queueCreateInfos.size());
    _logicalDeviceCreateInfo.pEnabledFeatures = nullptr; // NOTE: use later
    _logicalDeviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(Extensions.size());
    _logicalDeviceCreateInfo.ppEnabledExtensionNames = Extensions.data();


    if (_enableValidationLayers) {
      _logicalDeviceCreateInfo.enabledLayerCount = Layers.size();
      _logicalDeviceCreateInfo.ppEnabledLayerNames = Layers.data();
    } else {
      _logicalDeviceCreateInfo.enabledLayerCount = 0;
      _logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr;
    }

    VkResult result =
      vkCreateDevice(PhysicalDevice.Handle, &_logicalDeviceCreateInfo, nullptr,
                     &Handle);
    if (result != VK_SUCCESS) {
      SimpleMessageBox::ShowError(
          "Vulkan Logical Device",
          fmt::format(
              "Unable to create Vulkan Logical Device.\nVulkan Error Code: [{}]",
              result));
    }

    vkGetDeviceQueue(Handle, PhysicalDevice.QueueFamilies.graphicsFamily.value(), 0,
                   &GraphicsQueue);
    vkGetDeviceQueue(Handle, PhysicalDevice.QueueFamilies.presentFamily.value(), 0,
                   &PresentQueue);
  }

  void SetUp() {
    std::set<uint32_t> uniqueQueueFamilies = {PhysicalDevice.QueueFamilies.graphicsFamily.value(),
                                            PhysicalDevice.QueueFamilies.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      _queueCreateInfos.push_back(queueCreateInfo);
    }
  }

  VulkanPhysicalDevice PhysicalDevice;
  VkDevice Handle;
  VkQueue GraphicsQueue;
  VkQueue PresentQueue;
  std::vector<const char *> Extensions;
  std::vector<const char *> Layers;

  private:
  bool _enableValidationLayers;
  std::vector<VkDeviceQueueCreateInfo> _queueCreateInfos;
  VkDeviceCreateInfo _logicalDeviceCreateInfo;
};

#endif