cmake_minimum_required(VERSION 3.18.0)

set(VCPKG_MANIFEST_MODE TRUE)

set(C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
project(HelloVulkan VERSION 0.1.0)

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

add_executable(HelloVulkan WIN32 MACOSX_BUNDLE
  src/main.cpp
  src/Vulkan.cpp
  src/Application.cpp
  src/VulkanShaderManager.cpp
  src/VulkanPhysicalDevice.cpp
  src/KeyboardEvent.cpp
  src/Triangle.cpp
  src/VulkanMesh.cpp
  src/Camera.cpp
  src/Suzanne.cpp
  src/VkUtils.cpp
  src/VulkanAllocator.cpp
  src/VkImGui.cpp
  src/VkInitializers.cpp
  ${EDITOR_SRC}
)
target_include_directories(HelloVulkan PRIVATE include)
target_include_directories(HelloVulkan PRIVATE ${STB_INCLUDE_DIRS})

if (VULKAN_FOUND)
  message(STATUS "Found Vulkan... ${Vulkan_LIBRARIES}")
  target_link_libraries(HelloVulkan PRIVATE ${Vulkan_LIBRARIES}
    glm::glm
    SDL2::SDL2
    SDL2::SDL2main
    SDL2::SDL2-static
    fmt::fmt-header-only
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    tinyobjloader::tinyobjloader
    imgui::imgui
    SDL2::SDL2_image
  )
  target_include_directories(HelloVulkan PUBLIC ${Vulkan_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Unable to find VulkanSDK. Please confirm you have Vulkan properly set up for this environment.")
endif()

add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)
# Needed for Vulkan Z-range [0,1] rather than OpenGL [-1,1]
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_LEFT_HANDED)
add_compile_definitions(GLM_FORCE_RADIANS)

if (APPLE)
  set_target_properties(HelloVulkan PROPERTIES
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
