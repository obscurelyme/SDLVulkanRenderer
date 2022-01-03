#include "Application.hpp"

#include <SDL2/SDL_image.h>
#include <fmt/core.h>

#include <iostream>
#include <utility>

#include "Camera.hpp"
#include "DeltaTime.hpp"
#include "Editor/MainMenuBar.hpp"
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
  window = CoffeeMaker::Window::CreateWindow(appName, windowWidth, windowHeight);
  CoffeeMaker::Window::SetWindowIcon(icon);
  SDL_FreeSurface(icon);

  Camera::CreateMainCamera(windowWidth, windowHeight);
  renderer = new Vulkan(appName, window);
  VulkanImGui::Init(window, renderer);
}

Application::~Application() {
  VulkanImGui::Destroy();
  delete renderer;
  CoffeeMaker::Window::DestroyWindow();
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
          renderer->FramebufferResize();
        }
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          renderer->FramebufferResize();
        }
      }
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDLKeyboardEventManager::HandleKeyboardEvent(event.key);
      }
    }

    CoffeeMaker::DeltaTime::CurrentTime = SDL_GetTicks();
    CoffeeMaker::DeltaTime::SetDelta();

    VulkanImGui::NewFrame();
    VulkanImGui::Update();
    ImGui::ShowDemoWindow();

    CoffeeMaker::Editor::MainMenuBar::Render(window);

    renderer->Draw();
    CoffeeMaker::DeltaTime::PreviousTime = CoffeeMaker::DeltaTime::CurrentTime;
  }
}
