#ifndef _coffeemaker_renderer_vulkan_swapchain_hpp
#define _coffeemaker_renderer_vulkan_swapchain_hpp

#include <vulkan/vulkan.h>

#include <vector>

#include "Renderer/Vulkan/MemoryAllocator.hpp"

namespace CoffeeMaker::Renderer::Vulkan {

  class Swapchain {
    public:
    static void CreateSwapchain();
    static VkSwapchainKHR GetSwapchain();
    static void Destroy();
    static void SetPresentMode(VkPresentModeKHR mode);

    Swapchain() = default;
    ~Swapchain() = default;
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    private:
    void InitChooseSwapSurfaceFormat();
    void InitChoosePresentMode();
    void InitChooseSwapExtent();
    void InitCreateImageViews();
    void InitCreateDepthImageView();
    void InitCreateSwapchainKHR();

    public:
    static Swapchain* gSwapchain;
    static Swapchain* gPrevSwapchain;

    VkSwapchainKHR pSwapchain{VK_NULL_HANDLE};
    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{VK_PRESENT_MODE_IMMEDIATE_KHR};
    VkExtent2D extent{.width = 0, .height = 0};
    uint32_t imageCount{0};
    std::vector<VkImage> swapChainImages{};
    std::vector<VkImageView> swapChainImageViews{};

    // NOTE: Set up for Depth
    VkImageView depthImageView{VK_NULL_HANDLE};
    CoffeeMaker::Renderer::Vulkan::AllocatedImage depthImage{};
    VkFormat depthFormat{VK_FORMAT_D32_SFLOAT};
  };

}  // namespace CoffeeMaker::Renderer::Vulkan

#endif
