// core
#include "Camera.h"

Camera::Camera() {
    float x = 0;
    float y = 0;
    mPosition_ = glm::vec3(x, y, 4);
    mForward_ = glm::vec3(0.0f, 0.0f, 1.0f);
    mUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    mWorldUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    mRight_ = glm::normalize(glm::cross(mForward_, mWorldUp_));
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
    // TODO why negative here?
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
