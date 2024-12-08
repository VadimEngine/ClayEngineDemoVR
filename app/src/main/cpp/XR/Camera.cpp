#include "Camera.h"

namespace VR {

Camera::Camera() {
    float x = 0;
    float y = 0;
    mPosition_ = glm::vec3(x, y, 1);
    mForward_ = glm::vec3(0.0f, 0.0f, -1.0f);
    mUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    mWorldUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    mRight_ = glm::normalize(glm::cross(mForward_, mWorldUp_));
    mOrientation_ = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
    updateCameraVectors();
}

glm::mat4 Camera::getProjectionMatrix() const {
    // TODO get the width and height from somewhere instead of magic numbers
    return glm::perspective(glm::radians(mFOV_), (float)800 / (float)600, 0.1f, 100.0f);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(mPosition_, mPosition_ + mForward_, mUp_);
}

void Camera::updateCameraVectors() {
    // calculate the new Front vector
    glm::vec4 forward = {0,0,-1,0};
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(mRotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));

    forward =  rotationMatrix * forward;

    mForward_ = glm::normalize(forward);

    // also re-calculate the Right and Up vector
    mRight_ = glm::normalize(glm::cross(mForward_, mWorldUp_));
    mUp_ = glm::normalize(glm::cross(mRight_, mForward_));
}

void Camera::move(const glm::vec3 dir, const float step) {
    mPosition_ += (glm::normalize(dir) * step);
}

glm::vec3 Camera::getPosition() const {
    return mPosition_;
}

void Camera::setPosition(glm::vec3 newPos) {
    mPosition_ = newPos;
}

glm::vec3 Camera::getRotation() const {
    return mRotation_;
}

glm::quat Camera::getOrientation() const {
    return mOrientation_;
}

void Camera::rotateWithJoystick(const glm::vec2& joystickInput) {
    // Sensitivity factor to control the speed of rotation
    const float sensitivity = 1.0f;

    // Update the camera's rotation based on the joystick input
    mRotation_.x -= joystickInput.y * sensitivity;  // Up/Down rotation (pitch)
    mRotation_.y += joystickInput.x * sensitivity;  // Left/Right rotation (yaw)

    // Clamp the pitch (to prevent flipping the camera upside down)
    if (mRotation_.x > 89.0f) {
        mRotation_.x = 89.0f;
    }
    if (mRotation_.x < -89.0f) {
        mRotation_.x = -89.0f;
    }

    // Call updateCameraVectors to recalculate the camera's forward, right, and up vectors
    updateCameraVectors();
}
void Camera::updateWithJoystickInput(const glm::vec2& leftJoystick, const glm::vec2& rightJoystick, float moveSpeed, float rotationSpeed, const XrPosef& headPose) {
    // Threshold to consider joystick input as neutral
    const float joystickDeadZone = 0.1f;

    // Check if right joystick has movement (for orientation update)
    if (glm::length(rightJoystick) > joystickDeadZone) {
        // Normalize the joystick input to avoid speed increase with diagonal movement
        glm::vec2 normalizedRightJoystick = glm::normalize(rightJoystick);

        // Update orientation based on normalized right joystick input
        float pitch = 0;//-normalizedRightJoystick.y * rotationSpeed; // Rotate up/down (pitch)
        float yaw = normalizedRightJoystick.x * rotationSpeed;   // Rotate left/right (yaw)

        glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch around X-axis
        glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));     // Yaw around Y-axis

        mOrientation_ = glm::normalize(yawQuat * mOrientation_ * pitchQuat);
    }

    // Check if left joystick has movement (for position update)
    if (glm::length(leftJoystick) > joystickDeadZone) {
        // Normalize the joystick input to avoid speed increase with diagonal movement
        glm::vec2 normalizedLeftJoystick = glm::normalize(leftJoystick);

        glm::quat headPoseQuat = glm::quat(headPose.orientation.w, headPose.orientation.x, headPose.orientation.y, headPose.orientation.z);

        glm::quat combined = mOrientation_ * glm::conjugate(headPoseQuat);

        // Extract the roll y rotation) from the quaternion
        float yaw = -atan2(2.0f * (combined.w * combined.y - combined.z * combined.x),
                          1.0f - 2.0f * (combined.y * combined.y + combined.x * combined.x));


        // Create a rotation matrix for the roll (Z-axis rotation)
        glm::mat4 rollRotation = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));

        // Base forward and right vectors
        glm::vec3 baseForward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 baseRight = glm::vec3(1.0f, 0.0f, 0.0f);

        // Rotate the vectors using the roll rotation
        glm::vec3 forward = glm::normalize(glm::vec3(rollRotation * glm::vec4(baseForward, 1.0f)));
        glm::vec3 right = glm::normalize(glm::vec3(rollRotation * glm::vec4(baseRight, 1.0f)));


        forward = glm::normalize(forward);
        right = glm::normalize(right);

        // Update position based on joystick input
        mPosition_ += forward * normalizedLeftJoystick.y * moveSpeed; // Forward/backward
        mPosition_ += right * normalizedLeftJoystick.x * moveSpeed;   // Strafe left/right
    }
}

} // namespace VR
