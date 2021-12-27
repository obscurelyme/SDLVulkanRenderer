#include "Application.hpp"

#include <fmt/core.h>

#include <iostream>

#include "VulkanShaderManager.hpp"

Application::Application(const std::string &name, int windowWidth, int windowHeight) :
    appName(name), window(nullptr), renderer(nullptr), event({}), quit(false) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    // Error
  }

  window = SDL_CreateWindow(appName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth,
                            windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
  renderer = new Vulkan(appName, window);
}

Application::~Application() {
  delete renderer;
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Application::Run() {
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          std::cout << fmt::format("Window resized: ({},{})", event.window.data1, event.window.data2) << std::endl;
          renderer->FramebufferResize();
        }
      }
    }

    renderer->Draw2();

    SDL_Delay(16);
  }
}
