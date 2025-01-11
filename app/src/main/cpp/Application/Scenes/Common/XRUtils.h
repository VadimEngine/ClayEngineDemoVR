#pragma once
// third party
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <openxr/openxr.h>

namespace xr::utils {
    glm::mat4 computeViewMatrix(const XrPosef& pose);

    glm::mat4 computeViewMatrix(const XrPosef& pose, glm::vec3 cameraPos, glm::quat cameraOrientation);

    glm::mat4 computeProjectionMatrix(const XrFovf& fov, float nearZ, float farZ);
} // namespace xr::utils

