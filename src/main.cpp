#include "Application.hpp"

int main(int, char **) {
  Application app{"Vulkan Demo", 500, 500};
  app.Run();
  return 0;
}
