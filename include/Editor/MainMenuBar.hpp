#ifndef _coffeemaker_editor_mainmenu_hpp
#define _coffeemaker_editor_mainmenu_hpp

#include <SDL2/SDL.h>
#include <imgui.h>

#include "Renderer/Vulkan/PhysicalDevice.hpp"
#include "Vulkan.hpp"

namespace CoffeeMaker::Editor {
  class MainMenuBar {
    using PhysicalDevice = CoffeeMaker::Renderer::Vulkan::PhysicalDevice;

    public:
    static void Render(SDL_Window* window) {
      if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
          if (ImGui::BeginMenu("GPUs")) {
            for (auto gpu : PhysicalDevice::EnumeratePhysicalDevices(Vulkan::GetRenderer()->vulkanInstance)) {
              ImGui::MenuItem(gpu->Name(), nullptr, gpu->IsSelected());
            }
            ImGui::EndMenu();
          }

          if (ImGui::BeginMenu("Resolutions")) {
            if (ImGui::MenuItem("1920x1080")) {
              std::cout << "Setting window size to 1920x1080" << std::endl;
              SDL_SetWindowFullscreen(window, 0);
              SDL_SetWindowSize(window, 1920, 1080);
              SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            if (ImGui::MenuItem("1280x720")) {
              std::cout << "Setting window size to 1280x720" << std::endl;
              SDL_SetWindowFullscreen(window, 0);
              SDL_SetWindowSize(window, 1280, 720);
              SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            if (ImGui::MenuItem("800x600")) {
              std::cout << "Setting window size to 800x600" << std::endl;
              SDL_SetWindowFullscreen(window, 0);
              SDL_SetWindowSize(window, 800, 600);
              SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            ImGui::EndMenu();
          }

          if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
            SDL_Event event{};
            event.type = SDL_QUIT;
            SDL_PushEvent(&event);
          }
          ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
      }
    }
  };
}  // namespace CoffeeMaker::Editor

#endif
