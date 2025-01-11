cmake_minimum_required(VERSION 3.6)

project(ClayEngineDemoVR)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all .cpp files in the app's cpp directory
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/main/cpp/*.cpp)

# Add the app library
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

# Specify include directories for application code
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/main/cpp
)

# Set linker flags for ANativeActivity_onCreate
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")

set(CLAY_PLATFORM_VR ON CACHE BOOL "Set Platform to VR" FORCE)
set(CLAY_ENABLE_OPENGL_ES ON CACHE BOOL "Set Platform to Desktop" FORCE)
# Add ClayEngine
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/clay)

# Link against ClayEngine
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ClayEngine
)
