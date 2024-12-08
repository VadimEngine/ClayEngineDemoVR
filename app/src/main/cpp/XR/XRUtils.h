#pragma once
// standard lib
#include <string>
#include <vector>
// third party
#include <android/asset_manager.h>
#include <android/bitmap.h>
#include <android/imagedecoder.h>
#include <android/log.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <openxr/openxr.h>


#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, "XRApp", __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, "XRApp", __VA_ARGS__)
#define LOG_W(...) __android_log_print(ANDROID_LOG_WARN, "XRApp", __VA_ARGS__)

#define OPENXR_CHECK(x, y)                                                                          \
{                                                                                                   \
    XrResult result = (x);                                                                          \
    if (!XR_SUCCEEDED(result)) {                                                                    \
        LOG_E("OPENXR Error: %d", int(result));                                                     \
    }                                                                                               \
}

namespace xr::utils {
    glm::mat4 computeViewMatrix(const XrPosef& pose);

    glm::mat4 computeViewMatrix(const XrPosef& pose, glm::vec3 cameraPos, glm::quat cameraOrientation);

    glm::mat4 computeProjectionMatrix(const XrFovf& fov, float nearZ, float farZ);

    std::vector<char> loadAsset(AAssetManager* assetManager, const std::string& path);

    void loadImageAsset(AAssetManager* assetManager,
                        const char* filePath,
                        uint8_t** outPixels,
                        int& outWidth,
                        int& outHeight,
                        AndroidBitmapFormat& outFormat);

} // namespace xr::utils

