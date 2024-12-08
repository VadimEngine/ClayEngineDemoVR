cmake_minimum_required(VERSION 3.6)

project(ClayEngineDemoVR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define OpenXR loader path
set(OPENXR_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/OpenXR-SDK-Source)

# Add OpenXR loader source and include directories
add_subdirectory(${OPENXR_SDK_SOURCE_DIR} openxr)
# Add OpenAL Soft subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/openal-soft openal_build)
# Add libsndfile
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/libsndfile libsndfile_build)
# Add Assimp subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/assimp assimp_build)
# Add FreeType subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/freetype freetype_build)

# Collect all .cpp files in the cpp directory
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/main/cpp/*.cpp)

# Collect all .cpp files in the cpp directory
add_library(${PROJECT_NAME} SHARED
    ${SRC_FILES}
    # Add ImGui source files and Android native app glue
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_android.cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

# Specify include directories for third-party libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/main/cpp
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends
    ${CMAKE_SOURCE_DIR}/ThirdParty/glm
)

# export ANativeActivity_onCreate for java to call.
set_property(
    TARGET ${PROJECT_NAME}
    APPEND_STRING
    PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate"
)

# native_app_glue
include(AndroidNdkModules)
android_ndk_import_module_native_app_glue()

# Link libraries to your native library
target_link_libraries(${PROJECT_NAME} PRIVATE
    android
    EGL
    GLESv3
    log
    openxr_loader
    native_app_glue
    jnigraphics
    OpenAL
    sndfile
    assimp
    freetype
)