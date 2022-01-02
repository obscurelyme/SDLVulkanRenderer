#ifndef _coffeemaker_window_hpp
#define _coffeemaker_window_hpp

#include <SDL2/SDL.h>

#include <string>

namespace CoffeeMaker {
  class Window {
    public:
    static SDL_Window* CreateWindow(std::string name, int w, int h);
    static SDL_Window* MainWindow();
    static void DestroyWindow();
    static void SetWindowIcon(SDL_Surface* icon);

    static SDL_Window* gWindow;
  };
}  // namespace CoffeeMaker

#endif