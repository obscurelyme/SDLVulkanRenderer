#include "Application.hpp"

auto main(int, char **) -> int {
  Application app{"Vulkan Demo", 1920, 1080};
  app.Run();
  return 0;
}
