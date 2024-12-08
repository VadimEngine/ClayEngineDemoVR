#pragma once
// third party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
// project
#include "XRUtils.h"

namespace VR {

class Camera {
public:
    float nearZ = 0.05f;
    float farZ = 100.0f;

    float m_viewHeightM = 1.5f;

private:
    glm::vec3 mWorldUp_;
    glm::vec3 mForward_;
    glm::vec3 mUp_;
    glm::vec3 mRight_;
    float mFOV_ = 45.0f;
    glm::vec3 mRotation_ = glm::vec3(0.0, 0.0f, 0.0f);

    glm::vec3 mPosition_;
    glm::quat mOrientation_;

public:
    Camera();

    glm::mat4 getProjectionMatrix() const;

    glm::mat4 getViewMatrix() const;

    void move(const glm::vec3 dir, const float step);


    void updateCameraVectors();

    void setPosition(glm::vec3 newPos);

    glm::vec3 getPosition() const;

    glm::vec3 getRotation() const;

    glm::quat getOrientation() const;

    void rotateWithJoystick(const glm::vec2& joystickInput);

    void updateWithJoystickInput(const glm::vec2& leftJoystick,
                                 const glm::vec2& rightJoystick,
                                 float moveSpeed,
                                 float rotationSpeed,
                                 const XrPosef& headOrientation);
};

} // namespace VR
