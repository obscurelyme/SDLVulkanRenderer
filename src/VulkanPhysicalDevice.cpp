#include "VulkanPhysicalDevice.hpp"

std::vector<VulkanPhysicalDevice> VulkanPhysicalDevice::PhysicalDevices{};
std::multimap<int, VulkanPhysicalDevice> VulkanPhysicalDevice::RatedPhysicalDevices{};