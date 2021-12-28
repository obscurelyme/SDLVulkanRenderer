#ifndef _coffeemaker_keyboard_event_hpp
#define _coffeemaker_keyboard_event_hpp

#include <SDL2/SDL.h>

#include <vector>

class SDLKeyboardEventManager;

class SDLKeyboardEventListener {
  protected:
  SDLKeyboardEventListener() { _listeners.push_back(this); }
  virtual ~SDLKeyboardEventListener() {
    for (auto it = _listeners.begin(); it != _listeners.end();) {
      if ((*it) == this) {
        *it = nullptr;
      } else {
        ++it;
      }
    }
  }

  public:
  virtual void OnKeyboardEvent(const SDL_KeyboardEvent& event) = 0;

  friend class SDLKeyboardEventManager;

  private:
  static std::vector<SDLKeyboardEventListener*> _listeners;
  static void ProcessKeyboardEvent(const SDL_KeyboardEvent& event) {
    // NOTE: process the listeners for the current frame.
    // any added listeners during this process loop are not counted.
    size_t currentSize = _listeners.size();

    for (size_t i = 0; i < currentSize; i++) {
      if (_listeners[i] != nullptr) {
        _listeners[i]->OnKeyboardEvent(event);
      }
    }
  }
  static void RemoveStaleListeners() {
    for (auto it = _listeners.begin(); it != _listeners.end();) {
      if ((*it) == nullptr) {
        it = _listeners.erase(it);
      } else {
        ++it;
      }
    }
  }
};

class SDLKeyboardEventManager {
  public:
  static void HandleKeyboardEvent(const SDL_KeyboardEvent& event) {
    SDLKeyboardEventListener::ProcessKeyboardEvent(event);
    ClearUserEvents();
  }
  static void ClearUserEvents() { SDLKeyboardEventListener::RemoveStaleListeners(); }

  private:
  std::vector<SDLKeyboardEventListener*> _listeners;
};

#endif