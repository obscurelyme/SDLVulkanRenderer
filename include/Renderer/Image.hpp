#ifndef _coffeemaker_renderer_image_hpp
#define _coffeemaker_renderer_image_hpp

#include <vulkan/vulkan.h>

#include <string>

#include "Renderer/Vulkan/MemoryAllocator.hpp"

namespace CoffeeMaker::Renderer {

  class Texture {
    public:
    Texture() = default;
    ~Texture();
    Texture(const Texture& texture) = delete;
    Texture& operator=(const Texture& texture) = delete;

    int width{0};
    int height{0};
    int channels{0};
    std::string filename{""};
    VkDeviceSize size{0};
    VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
    CoffeeMaker::Renderer::Vulkan::AllocatedBuffer buffer{};
  };

  Texture* LoadTexture(const std::string& filename);

}  // namespace CoffeeMaker::Renderer

#endif
