#include "Camera.hpp"

#include <fmt/core.h>

#include <iostream>

std::shared_ptr<Camera> Camera::_instance = nullptr;

std::shared_ptr<Camera> Camera::MainCamera() {
  if (_instance == nullptr) {
    _instance = std::make_shared<Camera>(CameraType::Perspective);
  }
  return _instance;
}

Camera::Camera(CameraType t) : _type(t) {
  orthographicProj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 200.0f);
  perspectiveProj = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 0.1f, 200.0f);
  // perspectiveProj[1][1] *= -1;
  view = glm::translate(glm::mat4{1.0f}, position);
}

Camera::~Camera() = default;

void Camera::SetCameraType(CameraType t) { _type = t; }

glm::mat4 Camera::ScreenSpaceMatrix(glm::mat4 model) {
  if (_type == CameraType::Perspective) {
    return perspectiveProj * view * model;
  }
  // Orthographic camera
  return orthographicProj * view * model;
}

void Camera::OnKeyboardEvent(const SDL_KeyboardEvent& event) {
  if (event.keysym.scancode == SDL_SCANCODE_P && event.type == SDL_KEYUP) {
    SetCameraType(CameraType::Perspective);
  }

  if (event.keysym.scancode == SDL_SCANCODE_O && event.type == SDL_KEYUP) {
    SetCameraType(CameraType::Orthographic);
  }

  if (event.keysym.scancode == SDL_SCANCODE_UP && event.type == SDL_KEYDOWN) {
    position.z += 0.05f;
    view = glm::translate(glm::mat4{1.0f}, position);
    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_DOWN && event.type == SDL_KEYDOWN) {
    position.z -= 0.05f;
    view = glm::translate(glm::mat4{1.0f}, position);
    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_R && event.type == SDL_KEYUP) {
    position.z = -2.0f;
    view = glm::translate(glm::mat4{1.0f}, position);
    std::cout << "Camera Position Reset: " << fmt::format("({},{},{})", position.x, position.y, position.z)
              << std::endl;
  }
}