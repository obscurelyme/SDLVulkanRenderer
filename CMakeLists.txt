cmake_minimum_required(VERSION 3.18.0)

set(VCPKG_MANIFEST_MODE TRUE)

set(C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
project(CoffeeRender VERSION 0.1.0)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(imgui CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)

set(EDITOR_SRC src/Editor/ImGuiEditorObject.cpp)
set(RENDERER_VULKAN_SRC
  # src/Renderer/Vulkan/Commands.cpp
  # src/Renderer/Vulkan/Framebuffer.cpp
  src/Renderer/Vulkan/Utilities.cpp
  src/Renderer/Vulkan/LogicalDevice.cpp
  src/Renderer/Vulkan/MemoryAllocator.cpp
  src/Renderer/Vulkan/Pipeline.cpp
  src/Renderer/Vulkan/RenderPass.cpp
  # src/Renderer/Vulkan/Swapchain.cpp
  # src/Renderer/Vulkan/Syncronization.cpp
  # src/Renderer/Vulkan/VK.cpp
  # src/Renderer/Material.cpp
  src/Renderer/Vertex.cpp
  src/Renderer/Image.cpp
)

add_executable(CoffeeRender WIN32 MACOSX_BUNDLE
  src/main.cpp
  src/Vulkan.cpp
  src/Application.cpp
  src/VulkanShaderManager.cpp
  src/VulkanPhysicalDevice.cpp
  src/KeyboardEvent.cpp
  src/Triangle.cpp
  # src/VulkanMesh.cpp
  src/Camera.cpp
  # src/Suzanne.cpp
  # src/VkUtils.cpp
  src/VkImGui.cpp
  src/VkInitializers.cpp
  ${EDITOR_SRC}
  src/DeltaTime.cpp
  src/Rectangle.cpp
  src/Window.cpp
  ${RENDERER_VULKAN_SRC}
)
target_include_directories(CoffeeRender PRIVATE include)
target_include_directories(CoffeeRender PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(CoffeeRender PRIVATE  ${OPENGL_INCLUDE_DIRS})

if (VULKAN_FOUND)
  message(STATUS "Found Vulkan... ${Vulkan_LIBRARIES}")
  target_link_libraries(CoffeeRender PRIVATE ${Vulkan_LIBRARIES}
    glm::glm
    SDL2::SDL2
    SDL2::SDL2main
    SDL2::SDL2-static
    fmt::fmt-header-only
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    tinyobjloader::tinyobjloader
    imgui::imgui
    SDL2::SDL2_image
    ${OPENGL_LIBRARIES}
  )
  target_include_directories(CoffeeRender PUBLIC ${Vulkan_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Unable to find VulkanSDK. Please confirm you have Vulkan properly set up for this environment.")
endif()


add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)
# Needed for Vulkan Z-range [0,1] rather than OpenGL [-1,1]
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_LEFT_HANDED)
add_compile_definitions(GLM_FORCE_RADIANS)

if (APPLE)
  set_target_properties(CoffeeRender PROPERTIES
    BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER obscurelyme.vulkan-demo
    MACOSX_BUNDLE_BUNDLE_NAME "Vulkan Demo"
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0
    MACOSX_BUNDLE_BUNDLE_SHORT_VERSION_STRING "1.0.0"
  )
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if (APPLE)
  install(DIRECTORY "resources/vulkan" DESTINATION "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Resources/")
  install(FILES "vendor/vulkan/macos/libMoltenVK.dylib" "vendor/vulkan/macos/libvulkan.dylib" DESTINATION "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/")
  install(CODE "
    execute_process(COMMAND sh -c \"cp ./shaders/*.spv ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Resources\" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  ")
endif()
