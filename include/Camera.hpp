#ifndef _coffeemaker_camera_hpp
#define _coffeemaker_camera_hpp

// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

#include "KeyboardEvent.hpp"

enum class CameraType { Orthographic, Perspective };

class Camera : public SDLKeyboardEventListener {
  public:
  explicit Camera(CameraType t);
  ~Camera();

  static std::shared_ptr<Camera> MainCamera();
  void SetCameraType(CameraType t);

  glm::mat4 ScreenSpaceMatrix(glm::mat4 model);

  void OnKeyboardEvent(const SDL_KeyboardEvent& event);

  private:
  static std::shared_ptr<Camera> _instance;

  CameraType _type;
  glm::mat4 orthographicProj{1.0f};
  glm::mat4 perspectiveProj{1.0f};
  glm::mat4 view{1.0f};
  glm::vec3 position{0.0f, 0.0f, -2.0f};
};

#endif