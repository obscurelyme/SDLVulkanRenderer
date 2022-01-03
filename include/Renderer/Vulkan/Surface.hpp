#ifndef _coffeemaker_renderer_vulkan_surface_hpp
#define _coffeemaker_renderer_vulkan_surface_hpp

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

namespace CoffeeMaker::Renderer::Vulkan {

  class Surface {
    public:
    static void CreateSurface(SDL_Window* window, VkInstance instance);
    static VkSurfaceKHR GetSurface();
    static void Destroy();

    static VkSurfaceKHR gSurface;
    static VkInstance gVulkanInstance;
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
