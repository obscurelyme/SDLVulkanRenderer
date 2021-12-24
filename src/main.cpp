#include "Application.hpp"

int main(int, char **) {
  Application app{"Vulkan Demo", 800, 600};
  app.Run();
  return 0;
}
