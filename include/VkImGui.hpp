#ifndef _vulkan_imgui_hpp
#define _vulkan_imgui_hpp

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

#include "Editor/ImGuiEditorObject.hpp"
#include "Vulkan.hpp"

class VulkanImGui {
  public:
  static void Init(SDL_Window* mainWindow, Vulkan* mainRenderer) {
    window = mainWindow;
    renderer = mainRenderer;

    logicalDevice = renderer->logicalDevice.Handle;

    /**
     * STEP #1: create descriptor pool for IMGUI
     * the size of the pool is very oversize, but it's copied from imgui demo itself.
     */
    std::vector<VkDescriptorPoolSize> poolSizes{{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolCreateInfo.maxSets = 1000;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(logicalDevice, &poolCreateInfo, nullptr, &imguiPool);

    /**
     * STEP #2: initialize ImGui
     */

    // Initialize core structures for imgui
    ImGui::CreateContext();
    // initialize imgui for sdl
    ImGui_ImplSDL2_InitForVulkan(window);

    // initialize imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = renderer->vulkanInstance;
    init_info.PhysicalDevice = renderer->physicalDevice.Handle;
    init_info.Device = logicalDevice;
    init_info.Queue = renderer->logicalDevice.GraphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, renderer->renderPass.Handle);

    // execute a gpu command to upload imgui font textures
    renderer->ImmediateSubmit([&](VkCommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  static void Destroy() {
    std::cout << "=============== IMGUI DESTROY ============================" << std::endl;
    vkDestroyDescriptorPool(logicalDevice, imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    std::cout << "=============== IMGUI DESTROY ============================" << std::endl;
  }

  static void NewFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
  }

  static void Update() { CoffeeMaker::Editor::ImGuiEditorObjectManager::ImGuiEditorUpdate(); }

  static VkDescriptorPool imguiPool;
  static VkDevice logicalDevice;
  static SDL_Window* window;
  static Vulkan* renderer;
};

#endif