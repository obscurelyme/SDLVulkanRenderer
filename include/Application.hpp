#ifndef _coffeemaker_application_hpp
#define _coffeemaker_application_hpp

#include <string>

#include "SDL2/SDL.h"
#include "Vulkan.hpp"
#include "Window.hpp"

class Application {
  public:
  Application(std::string name, int width, int height);
  ~Application();

  void Run();

  private:
  std::string appName{"Application"};
  SDL_Window *window{nullptr};
  SDL_Surface *icon{nullptr};
  Vulkan *renderer{nullptr};
  SDL_Event event{};
  bool quit{false};
};

#endif
