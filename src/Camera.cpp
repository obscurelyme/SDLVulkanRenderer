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
  orthographicProj = glm::ortho(-400.0f, 400.0f, 300.0f, -300.0f, -200.0f, 200.0f);
  perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
  perspectiveProj[1][1] *= -1;
  view = glm::translate(glm::mat4{1.0f}, position);
}

Camera::~Camera() = default;

void Camera::SetCameraType(CameraType t) { _type = t; }

glm::mat4 Camera::ScreenSpaceMatrix(glm::mat4 model) {
  if (_num > 360.0f) {
    _num = 0;
  }
  _num++;
  if (_type == CameraType::Perspective) {
    view = glm::translate(glm::mat4{1.0f}, position);
    view = glm::rotate(view, glm::radians(_num), glm::vec3{0.0f, 1.0f, 0.0f});
    return perspectiveProj * view * model;
  }
  // Orthographic camera
  view = glm::translate(glm::mat4{1.0f}, position);
  view = glm::rotate(view, glm::radians(_num), glm::vec3{0.0f, 1.0f, 0.0f});
  view = glm::scale(view, glm::vec3{100.f, 100.f, 100.f});
  return orthographicProj * view * model;
}

void Camera::OnKeyboardEvent(const SDL_KeyboardEvent& event) {
  if (event.keysym.scancode == SDL_SCANCODE_P && event.type == SDL_KEYUP) {
    SetCameraType(CameraType::Perspective);
    std::cout << "Camera Mode: Perspective" << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_O && event.type == SDL_KEYUP) {
    SetCameraType(CameraType::Orthographic);
    std::cout << "Camera Mode: Orthographic" << std::endl;
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

  if (event.keysym.scancode == SDL_SCANCODE_A && event.type == SDL_KEYDOWN) {
    position.y += 0.05f;

    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_D && event.type == SDL_KEYDOWN) {
    position.y -= 0.05f;

    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_W && event.type == SDL_KEYDOWN) {
    position.x += 0.05f;

    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_S && event.type == SDL_KEYDOWN) {
    position.x -= 0.05f;

    std::cout << "Camera Position: " << fmt::format("({},{},{})", position.x, position.y, position.z) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_R && event.type == SDL_KEYUP) {
    position = glm::vec3{0.0f, 0.0f, -2.0f};
    view = glm::translate(glm::mat4{1.0f}, position);
    std::cout << "Camera Position Reset: " << fmt::format("({},{},{})", position.x, position.y, position.z)
              << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_RIGHT && event.type == SDL_KEYUP) {
    _fov++;
    perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
    perspectiveProj[1][1] *= -1;
    std::cout << "Camera Field of View: " << fmt::format("{}", _fov) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_F && event.type == SDL_KEYUP) {
    _fov = 70.0f;
    perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
    perspectiveProj[1][1] *= -1;
    std::cout << "Camera Field of View Reset: " << fmt::format("{}", _fov) << std::endl;
  }

  if (event.keysym.scancode == SDL_SCANCODE_LEFT && event.type == SDL_KEYUP) {
    _fov--;
    perspectiveProj = glm::perspective(glm::radians(_fov), 800.0f / 600.0f, 0.1f, 200.0f);
    perspectiveProj[1][1] *= -1;
    std::cout << "Camera Field of View: " << fmt::format("{}", _fov) << std::endl;
  }
}