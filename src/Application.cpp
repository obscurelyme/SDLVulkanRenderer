#include "Application.hpp"

#include <SDL2/SDL_image.h>
#include <fmt/core.h>

#include <iostream>
#include <utility>

#include "Camera.hpp"
#include "Triangle.hpp"
#include "VkImGui.hpp"
#include "VulkanShaderManager.hpp"

Application::Application(std::string name, int windowWidth, int windowHeight) : appName(std::move(name)) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    // Error
    std::cout << "SDL Failed to initialize" << SDL_GetError() << std::endl;
    abort();
  }

  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF) == 0) {
    // Error
    std::cout << "SDL_image Failed to initialize" << SDL_GetError() << std::endl;
    abort();
  }

  icon = IMG_Load(fmt::format("{}{}", SDL_GetBasePath(), "mug.png").c_str());
  window = SDL_CreateWindow(appName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth,
                            windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  SDL_SetWindowIcon(window, icon);
  Camera::CreateMainCamera(windowWidth, windowHeight);
  renderer = new Vulkan(appName, window);
  VulkanImGui::Init(window, renderer);
}

Application::~Application() {
  VulkanImGui::Destroy();
  delete renderer;
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

void Application::Run() {
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      // Handle ImGui events first...

      ImGui_ImplSDL2_ProcessEvent(&event);
      // Custom events next...

      if (event.type == SDL_QUIT) {
        quit = true;
      }
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          std::cout << fmt::format("Window resized: ({},{})", event.window.data1, event.window.data2) << std::endl;
          renderer->FramebufferResize();
        }
      }
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDLKeyboardEventManager::HandleKeyboardEvent(event.key);
      }
    }

    VulkanImGui::NewFrame();
    VulkanImGui::Update();

    renderer->Draw();

    SDL_Delay(16);
  }
}
