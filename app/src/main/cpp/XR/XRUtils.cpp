#include "XR/XRUtils.h"

namespace xr::utils {

glm::mat4 computeViewMatrix(const XrPosef& pose) {
    glm::quat rotation(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);
    glm::mat4 rotationMatrix = glm::mat4_cast(glm::conjugate(rotation)); // Conjugate to invert the rotation
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -glm::vec3(pose.position.x, pose.position.y, pose.position.z));
    glm::mat4 viewMatrix = rotationMatrix * translationMatrix;

    return viewMatrix;
}

glm::mat4 computeViewMatrix(const XrPosef& pose, glm::vec3 cameraPos, glm::quat cameraOrientation) {
    // Convert pose orientation (XrPosef) to a quaternion
    glm::quat poseRotation(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);
    glm::quat combinedRotation =  glm::conjugate(cameraOrientation) * poseRotation;

    // Compute the rotation matrix
    glm::mat4 rotationMatrix = glm::mat4_cast(glm::conjugate(combinedRotation));

    // Combine translations: Pose position and camera position
    glm::vec3 poseTranslation(pose.position.x, pose.position.y, pose.position.z);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -(poseTranslation + cameraPos));

    // Combine rotation and translation for the final view matrix
    glm::mat4 viewMatrix = rotationMatrix * translationMatrix;

    return viewMatrix;
}


glm::mat4 computeProjectionMatrix(const XrFovf& fov, float nearZ, float farZ) {
    glm::mat4 result(0.0f);

    const float tanAngleLeft = tan(fov.angleLeft);
    const float tanAngleRight = tan(fov.angleRight);
    const float tanAngleUp = tan(fov.angleUp);
    const float tanAngleDown = tan(fov.angleDown);

    const float tanAngleWidth = tanAngleRight - tanAngleLeft;
    const float tanAngleHeight = tanAngleUp - tanAngleDown;

    // For OpenGL, we use a clip space with positive Y up and Z in [-1, 1]
    const float offsetZ = nearZ;

    // Normal projection
    result[0][0] = 2.0f / tanAngleWidth;
    result[1][1] = 2.0f / tanAngleHeight;
    result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
    result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
    result[2][2] = -(farZ + offsetZ) / (farZ - nearZ);
    result[2][3] = -1.0f;
    result[3][2] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

    return result;
}

std::vector<char> loadAsset(AAssetManager* assetManager, const std::string& path) {
    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        throw std::runtime_error("Failed to load asset: " + std::string(path));
    }

    // Get the asset's size and read the data into memory
    size_t fileSize = AAsset_getLength(asset);
    std::vector<char> fileData(fileSize);
    AAsset_read(asset, fileData.data(), fileSize);
    AAsset_close(asset);

    return fileData;
}

void loadImageAsset(AAssetManager* assetManager,
                    const char* filePath,
                    uint8_t** outPixels, // try uint8_t*& instead
                    int& outWidth,
                    int& outHeight,
                    AndroidBitmapFormat& outFormat) {
    AAsset* asset = AAssetManager_open(assetManager, filePath, AASSET_MODE_STREAMING);

    if (!asset) {
        throw std::runtime_error("Failed to open asset file: " + std::string(filePath));
    }

    AImageDecoder* decoder = nullptr;
    int result = AImageDecoder_createFromAAsset(asset, &decoder);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        AAsset_close(asset);
        throw std::runtime_error("Failed to create image decoder for file: " + std::string(filePath));
    }

    const AImageDecoderHeaderInfo* info = AImageDecoder_getHeaderInfo(decoder);
    outWidth = AImageDecoderHeaderInfo_getWidth(info);
    outHeight = AImageDecoderHeaderInfo_getHeight(info);
    outFormat = (AndroidBitmapFormat) AImageDecoderHeaderInfo_getAndroidBitmapFormat(info);
    size_t stride = AImageDecoder_getMinimumStride(decoder);
    size_t size = outHeight * stride;

    *outPixels = (uint8_t*)malloc(size);
    if (!*outPixels) {
        AImageDecoder_delete(decoder);
        AAsset_close(asset);
        throw std::runtime_error("Failed to allocate memory for pixel data.");
    }

    result = AImageDecoder_decodeImage(decoder, *outPixels, stride, size);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        free(*outPixels);
        AImageDecoder_delete(decoder);
        AAsset_close(asset);
        throw std::runtime_error("Failed to decode image from asset.");
    }

    // Cleanup
    AImageDecoder_delete(decoder);
    AAsset_close(asset);
}

} // namespace xr::utils
