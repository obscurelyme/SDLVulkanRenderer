#ifndef _coffeemaker_imgui_editor_object_hpp
#define _coffeemaker_imgui_editor_object_hpp

#include <vector>

namespace CoffeeMaker::Editor {
  class ImGuiEditorObjectManager;

  class ImGuiEditorObject {
    public:
    friend class ImGuiEditorObjectManager;
    ImGuiEditorObject();
    virtual ~ImGuiEditorObject();

    virtual void EditorUpdate() = 0;

    private:
    static void ImGuiEditorUpdateObjects();
    static void ClearStaleObjects();
    static std::vector<ImGuiEditorObject*> objects;
  };

  class ImGuiEditorObjectManager {
    public:
    static void ImGuiEditorUpdate();
  };
};  // namespace CoffeeMaker::Editor

#endif