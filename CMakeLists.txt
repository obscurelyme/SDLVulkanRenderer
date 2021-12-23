cmake_minimum_required(VERSION 3.18.0)

set(VCPKG_MANIFEST_MODE TRUE)

set(C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
project(HelloVulkan VERSION 0.1.0)

find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(HelloVulkan WIN32 MACOSX_BUNDLE src/main.cpp src/Vulkan.cpp src/Application.cpp)
target_include_directories(HelloVulkan PRIVATE include)

if (VULKAN_FOUND)
  message(STATUS "Found Vulkan... ${Vulkan_LIBRARIES}")
  target_link_libraries(HelloVulkan PRIVATE ${Vulkan_LIBRARIES} SDL2::SDL2 SDL2::SDL2main SDL2::SDL2-static fmt::fmt-header-only)
  target_include_directories(HelloVulkan PUBLIC ${Vulkan_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Unable to find VulkanSDK. Please confirm you have Vulkan properly set up for this environment.")
endif()

add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)

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
endif()
