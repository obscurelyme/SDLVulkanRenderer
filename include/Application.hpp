#ifndef _coffeemaker_application_hpp
#define _coffeemaker_application_hpp

#include "SDL2/SDL.h"
#include "Vulkan.hpp"
#include <string>

class Application {
public:
  Application(const std::string &name, int width, int height);
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
