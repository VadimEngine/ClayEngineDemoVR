cmake_minimum_required(VERSION 3.6)

project(Template)

set(CMAKE_CXX_STANDARD 11) # TODO use higher c++
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define OpenXR loader path
set(OPENXR_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/OpenXR-SDK-Source)

# Add OpenXR loader source and include directories
add_subdirectory(${OPENXR_SDK_SOURCE_DIR} openxr)

# Collect all .cpp files in the cpp directory
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/main/cpp/*.cpp)

# Collect all .cpp files in the cpp directory
add_library(${CMAKE_PROJECT_NAME} SHARED
    ${SRC_FILES}
    # Add ImGui source files and Android native app glue
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_android.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

# Specify include directories for third-party libraries
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/main/cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends
    ${CMAKE_SOURCE_DIR}/ThirdParty/glm
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Link libraries to your native library
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    android
    EGL
    GLESv3
    log
    openxr_loader
)
