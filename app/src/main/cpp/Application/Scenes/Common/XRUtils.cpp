// standard lib
#include <string>
#include <vector>
// header
#include "Application/Scenes/Common/XRUtils.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

} // namespace xr::utils
