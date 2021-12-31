#ifndef _coffeemaker_camera_hpp
#define _coffeemaker_camera_hpp

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <vector>

#include "Editor/ImGuiEditorObject.hpp"
#include "KeyboardEvent.hpp"

enum class CameraType { Orthographic, Perspective };

class Camera : public SDLKeyboardEventListener, public CoffeeMaker::Editor::ImGuiEditorObject {
  public:
  explicit Camera(CameraType t);
  ~Camera();

  static void CreateMainCamera(uint32_t extentWidth, uint32_t extentHeight);
  static void SetMainCameraDimensions(uint32_t extentWidth, uint32_t extentHeight);
  static std::shared_ptr<Camera> MainCamera();
  void SetCameraType(CameraType t);

  void SetCameraDimensions(uint32_t extentWidth, uint32_t extentHeight);

  glm::mat4 ScreenSpaceMatrix(glm::mat4 model);

  void OnKeyboardEvent(const SDL_KeyboardEvent& event) override;

  void EditorUpdate() override;

  void OnCameraModeChange(std::function<void(CameraType)>);

  private:
  void EmitCameraModeEvent();

  static std::shared_ptr<Camera> _instance;
  static uint32_t width;
  static uint32_t height;

  CameraType _type;
  glm::mat4 orthographicProj{1.0f};
  glm::mat4 perspectiveProj{1.0f};
  glm::mat4 view{1.0f};
  glm::vec3 position{0.0f, 0.0f, 2.0f};
  float _fov{70.0f};
  float _num{0.0f};
  bool _ortho{false};
  float scale{1.0f};

  std::vector<std::function<void(CameraType)>> _listeners;
};

#endif
