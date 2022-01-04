#include "Renderer/Vulkan/PhysicalDevice.hpp"

#include "Renderer/Vulkan/Surface.hpp"

std::vector<CoffeeMaker::Renderer::Vulkan::PhysicalDevice*>
    CoffeeMaker::Renderer::Vulkan::PhysicalDevice::gPhysicalDevices{};

std::multimap<int, CoffeeMaker::Renderer::Vulkan::PhysicalDevice*>
    CoffeeMaker::Renderer::Vulkan::PhysicalDevice::gRatedPhysicalDevices{};

CoffeeMaker::Renderer::Vulkan::PhysicalDevice* CoffeeMaker::Renderer::Vulkan::PhysicalDevice::gPhysicalDeviceInUse{
    nullptr};

size_t CoffeeMaker::Renderer::Vulkan::PhysicalDevice::_uid{0};

std::vector<CoffeeMaker::Renderer::Vulkan::PhysicalDevice*>&
CoffeeMaker::Renderer::Vulkan::PhysicalDevice::EnumeratePhysicalDevices(VkInstance instance) {
  if (gPhysicalDevices.size() > 0) {
    return gPhysicalDevices;
  }

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

  if (physicalDeviceCount == 0) {
    SDL_LogError(0, "Unable to find Physical GPUs on this system that support Vulkan.");
    exit(3333);
  }

  std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());

  for (VkPhysicalDevice device : devices) {
    auto newPhysicalDevice = new PhysicalDevice(device);
    newPhysicalDevice->FindQueueFamilies();
    newPhysicalDevice->QuerySwapchainSupport();
    newPhysicalDevice->gPhysicalDevices.push_back(newPhysicalDevice);
  }

  return gPhysicalDevices;
}

void CoffeeMaker::Renderer::Vulkan::PhysicalDevice::ClearAllPhysicalDevices() {
  for (auto d : gPhysicalDevices) {
    delete d;
  }
  gPhysicalDevices.clear();
  gRatedPhysicalDevices.clear();
  gPhysicalDeviceInUse = nullptr;
}

VkPhysicalDevice CoffeeMaker::Renderer::Vulkan::PhysicalDevice::GetVkpPhysicalDeviceInUse() {
  return gPhysicalDeviceInUse->vkpPhysicalDevice;
}

CoffeeMaker::Renderer::Vulkan::PhysicalDevice* CoffeeMaker::Renderer::Vulkan::PhysicalDevice::GetPhysicalDeviceInUse() {
  return gPhysicalDeviceInUse;
}

void CoffeeMaker::Renderer::Vulkan::PhysicalDevice::SelectPhysicalDevice(size_t idToSelect) {
  if (gPhysicalDeviceInUse != nullptr) {
    gPhysicalDeviceInUse->selected = false;
  }

  for (size_t j = 0; j < gPhysicalDevices.size(); j++) {
    if (gPhysicalDevices[j]->id == idToSelect) {
      gPhysicalDevices[j]->selected = true;
      gPhysicalDeviceInUse = gPhysicalDevices[j];
      return;
    }
  }
}

CoffeeMaker::Renderer::Vulkan::PhysicalDevice::PhysicalDevice(VkPhysicalDevice device) {
  id = ++_uid;
  vkGetPhysicalDeviceProperties(device, &Properties);
  vkGetPhysicalDeviceFeatures(device, &Features);

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  SupportedExtensions.resize(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, SupportedExtensions.data());

  vkGetPhysicalDeviceMemoryProperties(device, &MemoryProperties);

  vkpPhysicalDevice = device;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::operator==(PhysicalDevice& rhs) {
  return strcmp(Name(), rhs.Name()) == 0;
}

const char* CoffeeMaker::Renderer::Vulkan::PhysicalDevice::Name() { return Properties.deviceName; }

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsDiscreteGPU() {
  return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsIntegratedGPU() {
  return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsVirtualGPU() {
  return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsCPU() {
  return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsOtherType() {
  return Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER;
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::IsSelected() { return selected; }

void CoffeeMaker::Renderer::Vulkan::PhysicalDevice::FindQueueFamilies() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkpPhysicalDevice, &queueFamilyCount, nullptr);

  QueueFamilies.properties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(vkpPhysicalDevice, &queueFamilyCount, QueueFamilies.properties.data());

  int i = 0;
  for (const auto& queueFamily : QueueFamilies.properties) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkpPhysicalDevice, i, Surface::GetSurface(), &presentSupport);

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

void CoffeeMaker::Renderer::Vulkan::PhysicalDevice::QuerySwapchainSupport() {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkpPhysicalDevice, Surface::GetSurface(), &SwapChainSupport.capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(vkpPhysicalDevice, Surface::GetSurface(), &formatCount, nullptr);
  if (formatCount != 0) {
    SwapChainSupport.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkpPhysicalDevice, Surface::GetSurface(), &formatCount,
                                         SwapChainSupport.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(vkpPhysicalDevice, Surface::GetSurface(), &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    SwapChainSupport.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkpPhysicalDevice, Surface::GetSurface(), &presentModeCount,
                                              SwapChainSupport.presentModes.data());
  }
}

bool CoffeeMaker::Renderer::Vulkan::PhysicalDevice::AreExtensionsSupported(
    std::vector<const char*>& requestedExtensions) {
  std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

  for (const auto& extension : SupportedExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
