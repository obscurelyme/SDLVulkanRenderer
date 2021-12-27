#include "Application.hpp"

auto main(int, char **) -> int {
  Application app{"Vulkan Demo", 800, 600};
  app.Run();
  return 0;
}
