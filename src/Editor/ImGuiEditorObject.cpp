#include "Editor/ImGuiEditorObject.hpp"

std::vector<CoffeeMaker::Editor::ImGuiEditorObject*> CoffeeMaker::Editor::ImGuiEditorObject::objects{};

CoffeeMaker::Editor::ImGuiEditorObject::ImGuiEditorObject() { objects.push_back(this); }

CoffeeMaker::Editor::ImGuiEditorObject::~ImGuiEditorObject() {
  for (auto it = objects.begin(); it != objects.end();) {
    if ((*it) == this) {
      *it = nullptr;
    } else {
      ++it;
    }
  }
}

void CoffeeMaker::Editor::ImGuiEditorObject::ImGuiEditorUpdateObjects() {
  const size_t size = objects.size();

  for (size_t i = 0; i < size; i++) {
    if (objects[i] != nullptr) {
      objects[i]->EditorUpdate();
    }
  }
}

void CoffeeMaker::Editor::ImGuiEditorObject::ClearStaleObjects() {
  for (auto it = objects.begin(); it != objects.end();) {
    if ((*it) == nullptr) {
      it = objects.erase(it);
    } else {
      ++it;
    }
  }
}

void CoffeeMaker::Editor::ImGuiEditorObjectManager::ImGuiEditorUpdate() {
  ImGuiEditorObject::ImGuiEditorUpdateObjects();
  ImGuiEditorObject::ClearStaleObjects();
}
