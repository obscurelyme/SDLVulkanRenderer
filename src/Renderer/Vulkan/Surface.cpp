#include "Renderer/Vulkan/Surface.hpp"

#include "SDL2/SDL_vulkan.h"

VkSurfaceKHR CoffeeMaker::Renderer::Vulkan::Surface::gSurface{VK_NULL_HANDLE};
VkInstance CoffeeMaker::Renderer::Vulkan::Surface::gVulkanInstance{VK_NULL_HANDLE};

void CoffeeMaker::Renderer::Vulkan::Surface::CreateSurface(SDL_Window* window, VkInstance instance) {
  if (!SDL_Vulkan_CreateSurface(window, instance, &gSurface)) {
    SDL_LogError(0, "SDL2 was unable to create Vulkan Surface.\nSDL Error: [%s]", SDL_GetError());
    exit(1111);
  }

  gVulkanInstance = instance;
}

VkSurfaceKHR CoffeeMaker::Renderer::Vulkan::Surface::GetSurface() { return gSurface; }

void CoffeeMaker::Renderer::Vulkan::Surface::Destroy() {
  vkDestroySurfaceKHR(gVulkanInstance, gSurface, nullptr);
  gSurface = VK_NULL_HANDLE;
}
