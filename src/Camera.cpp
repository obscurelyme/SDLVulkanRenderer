#include "Camera.hpp"

#include <fmt/core.h>

#include <iostream>

#include "imgui.h"

uint32_t Camera::width = 800;
uint32_t Camera::height = 600;
std::shared_ptr<Camera> Camera::_instance = nullptr;

std::shared_ptr<Camera> Camera::MainCamera() { return _instance; }

void Camera::CreateMainCamera(uint32_t extentWidth, uint32_t extentHeight) {
  width = extentWidth;
  height = extentHeight;
  if (_instance == nullptr) {
    _instance = std::make_shared<Camera>(CameraType::Orthographic);
  }
}

Camera::Camera(CameraType t) : _type(t) {
  orthographicProj =
      glm::ortho(-1.0f * (width / 2.0f), (width / 2.0f), (height / 2.0f), -1.0f * (height / 2.0f), -1000.0f, 1000.0f);
  // orthographicProj = glm::ortho(0.f, static_cast<float>(width), static_cast<float>(height), 0.f, -1000.0f, 1000.0f);
  perspectiveProj =
      glm::perspective(glm::radians(_fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 200.0f);
  // perspectiveProj[1][1] *= -1;
  orthographicProj[1][1] *= -1;
  view = glm::translate(glm::mat4{1.0f}, position);
}

Camera::~Camera() = default;

void Camera::SetCameraType(CameraType t) {
  _type = t;
  EmitCameraModeEvent();
}

glm::mat4 Camera::ScreenSpaceMatrix(glm::mat4 model) {
  if (_num > 360.0f) {
    _num = 0;
  }
  _num++;
  if (_type == CameraType::Perspective) {
    view = glm::translate(glm::mat4{1.0f}, position);
    // view = glm::rotate(view, glm::radians(_num), glm::vec3{0.0f, 1.0f, 0.0f});
    return perspectiveProj * view * model;
  }
  // Orthographic camera
  view = glm::translate(glm::mat4{1.0f}, position);
  view = glm::scale(view, glm::vec3{height / scale, height / scale, 1.0f});
  return orthographicProj * view * model;
}

void Camera::EditorUpdate() {
  ImGui::Begin("Camera");

  ImGui::InputFloat("xPos", &position.x, 1.0f, 5.0f, "%.0f");
  ImGui::InputFloat("yPos", &position.y, 1.0f, 5.0f, "%.0f");
  ImGui::InputFloat("zPos", &position.z, 1.0f, 5.0f, "%.0f");

  ImGui::SliderFloat("field of view", &_fov, 0.0f, 360.0f, "%.0f");
  if (ImGui::Button("Set fov")) {
    perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
    // perspectiveProj[1][1] *= -1;
  }

  if (ImGui::Button("Toggle Proj Mode")) {
    if (_type == CameraType::Orthographic) {
      SetCameraType(CameraType::Perspective);
    } else {
      SetCameraType(CameraType::Orthographic);
    }
  }
  ImGui::Text("Proj Mode: %s", _type == CameraType::Orthographic ? "Ortho" : "Perspect");

  ImGui::InputFloat("Camera View scale: %f", &scale, 1.0f, 10.f, "%.0f");

  ImGui::End();
}

void Camera::OnCameraModeChange(std::function<void(CameraType)> listener) { _listeners.push_back(listener); }

void Camera::EmitCameraModeEvent() {
  for (auto fn : _listeners) {
    fn(_type);
  }
}

void Camera::OnKeyboardEvent(const SDL_KeyboardEvent& event) {
  // if (event.keysym.scancode == SDL_SCANCODE_P && event.type == SDL_KEYUP) {
  //   SetCameraType(CameraType::Perspective);
  //   std::cout << "Camera Mode: Perspective" << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_O && event.type == SDL_KEYUP) {
  //   SetCameraType(CameraType::Orthographic);
  //   std::cout << "Camera Mode: Orthographic" << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_UP && event.type == SDL_KEYDOWN) {
  //   position.z -= 0.05f;
  //   view = glm::translate(glm::mat4{1.0f}, position);
  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_DOWN && event.type == SDL_KEYDOWN) {
  //   position.z += 0.05f;
  //   view = glm::translate(glm::mat4{1.0f}, position);
  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_W && event.type == SDL_KEYDOWN) {
  //   position.y += 0.05f;

  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_S && event.type == SDL_KEYDOWN) {
  //   position.y -= 0.05f;

  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_A && event.type == SDL_KEYDOWN) {
  //   position.x += 0.05f;

  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_D && event.type == SDL_KEYDOWN) {
  //   position.x -= 0.05f;

  //   std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_R && event.type == SDL_KEYUP) {
  //   position = glm::vec3{0.0f, 0.0f, 2.0f};
  //   view = glm::translate(glm::mat4{1.0f}, position);
  //   std::cout << "Camera Position Reset: " << fmt::format("({},{},{})", position.x, position.y, position.z)
  //             << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_RIGHT && event.type == SDL_KEYUP) {
  //   _fov++;
  //   perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
  //   perspectiveProj[1][1] *= -1;
  //   std::cout << "Camera Field of View: " << fmt::format("{}", _fov) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_F && event.type == SDL_KEYUP) {
  //   _fov = 70.0f;
  //   perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
  //   perspectiveProj[1][1] *= -1;
  //   std::cout << "Camera Field of View Reset: " << fmt::format("{}", _fov) << std::endl;
  // }

  // if (event.keysym.scancode == SDL_SCANCODE_LEFT && event.type == SDL_KEYUP) {
  //   _fov--;
  //   perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
  //   perspectiveProj[1][1] *= -1;
  //   std::cout << "Camera Field of View: " << fmt::format("{}", _fov) << std::endl;
  // }
}