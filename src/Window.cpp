#include "Window.hpp"

SDL_Window* CoffeeMaker::Window::gWindow = nullptr;

SDL_Window* CoffeeMaker::Window::CreateWindow(std::string name, int width, int height) {
  DestroyWindow();
  gWindow = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                             SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  return gWindow;
}

SDL_Window* CoffeeMaker::Window::MainWindow() { return gWindow; }

void CoffeeMaker::Window::DestroyWindow() {
  if (gWindow != nullptr) {
    SDL_DestroyWindow(gWindow);
  }
}

void CoffeeMaker::Window::SetWindowIcon(SDL_Surface* icon) { SDL_SetWindowIcon(gWindow, icon); }