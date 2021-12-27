#ifndef _coffeemaker_application_hpp
#define _coffeemaker_application_hpp

#include <string>

#include "SDL2/SDL.h"
#include "Vulkan.hpp"

class Application {
  public:
  Application(std::string name, int width, int height);
  ~Application();

  void Run();

  private:
  std::string appName;
  SDL_Window *window;
  Vulkan *renderer;
  SDL_Event event;
  bool quit;
};

#endif
