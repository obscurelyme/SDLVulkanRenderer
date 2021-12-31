#include "Application.hpp"

auto main(int, char **) -> int {
  Application app{"Vulkan Demo", 1280, 720};
  app.Run();
  return 0;
}
