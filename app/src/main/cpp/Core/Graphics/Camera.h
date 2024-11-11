#pragma once
// third party
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <openxr/openxr.h>


class Camera {
private:
    glm::vec3 mWorldUp_;
    glm::vec3 mPosition_;
    glm::vec3 mForward_;
    glm::vec3 mUp_;
    glm::vec3 mRight_;
    float mFOV_ = 45.0f;
    glm::vec3 mRotation_ = glm::vec3(0.0, 0.0f, 0.0f);

public:
    Camera();

    glm::mat4 getProjectionMatrix() const;

    glm::mat4 getViewMatrix() const;

    void move(const glm::vec3 dir, const float step);


    void updateCameraVectors();

};

