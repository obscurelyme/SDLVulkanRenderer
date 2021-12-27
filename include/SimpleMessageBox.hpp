#ifndef _coffeemaker_simple_message_box_hpp
#define _coffeemaker_simple_message_box_hpp

#include <SDL2/SDL.h>

#include <string>

class SimpleMessageBox {
  public:
  static void ShowWarning(const std::string& title, const std::string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title.c_str(), message.c_str(), nullptr);
  }

  static void ShowError(const std::string& title, const std::string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), nullptr);
    abort();
  }

  static void ShowMessage(const std::string& title, const std::string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), nullptr);
  }
};

#endif