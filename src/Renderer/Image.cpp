#include "Renderer/Image.hpp"

#include <SDL2/SDL.h>
#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

CoffeeMaker::Renderer::Texture* CoffeeMaker::Renderer::LoadTexture(const std::string& filename) {
  using namespace CoffeeMaker::Renderer::Vulkan;

  auto pTexture = new CoffeeMaker::Renderer::Texture();

  std::string fullFilename = fmt::format("{}{}", SDL_GetBasePath(), filename);
  int width, height, channels;
  stbi_uc* pixels = stbi_load(fullFilename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  pTexture->width = width;
  pTexture->height = height;
  pTexture->channels = channels;
  pTexture->filename = filename;
  pTexture->format = VK_FORMAT_R8G8B8A8_SRGB;
  pTexture->size = static_cast<VkDeviceSize>(width * height * 4);

  pTexture->buffer = CreateBuffer(pTexture->size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
  MapMemory(pixels, pTexture->size, pTexture->buffer.allocation);
  stbi_image_free(pixels);

  return pTexture;
}

CoffeeMaker::Renderer::Texture::~Texture() {
  using namespace CoffeeMaker::Renderer::Vulkan;

  UnmapMemory(buffer.allocation);
  DestroyBuffer(buffer);
}
