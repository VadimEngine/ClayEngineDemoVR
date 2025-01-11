// third party
#include <glm/gtc/matrix_transform.hpp>
// project
#include "Application/Scenes/Sandbox/SandboxScene.h"
#include "Application/Scenes/Space/SpaceScene.h"
// class
#include "SandboxGUI.h"

SandboxGUI::SandboxGUI(clay::ShaderProgram* pTextureShader, clay::Mesh* pPlaneMesh, SandboxScene* theScene)
    : mShader_(pTextureShader), mPlaneMesh_(pPlaneMesh), mpScene_(theScene) {

    mTextureDim_ = {4128, 2208}; // imgui uses this size for its fbo

    // Generate and bind framebuffer
    glGenFramebuffers(1, &mFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO_);

    // Generate a texture to store the framebuffer's contents
    glGenTextures(1, &mFBTextureId_);
    glBindTexture(GL_TEXTURE_2D, mFBTextureId_);

    // Allocate storage for the texture (RGBA format, unsigned byte type)
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            mTextureDim_.x,
            mTextureDim_.y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
    );

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFBTextureId_, 0);

    // Check for framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_E("Framebuffer is not complete!");
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

SandboxGUI::~SandboxGUI() = default;

void SandboxGUI::setInputHandler(clay::InputHandlerXR* pInputHandler) {
    mpInputHandler_ = pInputHandler;
}

void SandboxGUI::render(const glm::mat4& view, const glm::mat4& proj) {
    // cache previous frame buffer
    GLint previousFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    // Set to render onto texture frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear the framebuffer

    startRender();
    buildImGui(view, proj);
    endRender();

    // return back to previous frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
    renderPlane(view, proj);
}

void SandboxGUI::renderPlane(const glm::mat4& view, const glm::mat4& proj) {
    // render plane
    mShader_->bind();
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mFBTextureId_);
    mShader_->setInt("theTexture", 0);

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(mRotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), mScale_);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mPosition_);
    mShader_->setMat4("model", translationMatrix * rotationMatrix * scaleMatrix);

    mShader_->setMat4("view", view);
    mShader_->setMat4("projection",  proj);
    mPlaneMesh_->render(*mShader_);
}

void SandboxGUI::setPosition(const glm::vec3& position) {
    mPosition_ = position;
}

void SandboxGUI::setRotation(const glm::vec3& rotation) {
    mRotation_ = rotation;
    glm::vec4 normal4 = {0,1,0,0};

    glm::vec3 rotationRadians = glm::radians(rotation);

    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), rotationRadians.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), rotationRadians.y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), rotationRadians.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Combine all rotations into a single matrix
    glm::mat4 rotationMatrix = rotationMatrixZ * rotationMatrixY * rotationMatrixX;
    glm::vec4 rotatedNormal = rotationMatrix * normal4;

    mNormal_ = {rotatedNormal.x, rotatedNormal.y, rotatedNormal.z};
}

void SandboxGUI::setScale(const glm::vec3& scale) {
    mScale_ = scale;
}

void SandboxGUI::buildImGui(const glm::mat4& view, const glm::mat4& proj) {
    ImGuiIO& io = ImGui::GetIO();

    io.MouseDrawCursor = true;

    io.MousePos = calMousePos;

    const float rightTriggerState = mpInputHandler_->getTriggerState(clay::InputHandlerXR::Hand::RIGHT);

    if (rightTriggerState > .1f) {
        io.AddMouseButtonEvent(0, true);  // Simulate mouse down
    }

    if (rightTriggerState < .1f) {
        io.AddMouseButtonEvent(0, false);  // Simulate mouse up
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(mTextureDim_.x, mTextureDim_.y));  // Match framebuffer size

    ImGui::Begin("Plane");
    ImGui::SetWindowFontScale(6.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    static int counter = 0;
    if (ImGui::Button("Update Counter")) {
        ++counter;
    }
    ImGui::Text("Counter: %i", counter);
    ImGui::Separator();
    ImGui::Text("Grab: %f, %f", mpInputHandler_->getGrabState(clay::InputHandlerXR::Hand::LEFT), mpInputHandler_->getGrabState(clay::InputHandlerXR::Hand::RIGHT));
    ImGui::Text("Trigger: %f, %f", mpInputHandler_->getTriggerState(clay::InputHandlerXR::Hand::LEFT), mpInputHandler_->getTriggerState(clay::InputHandlerXR::Hand::RIGHT));


    ImGui::Text("Buttons: %i, %i, %i, %i",
                mpInputHandler_->getButtonDown(clay::InputHandlerXR::Button::Y),
                mpInputHandler_->getButtonDown(clay::InputHandlerXR::Button::X),
                mpInputHandler_->getButtonDown(clay::InputHandlerXR::Button::B),
                mpInputHandler_->getButtonDown(clay::InputHandlerXR::Button::A)
    );
    const auto& leftGripPose = mpInputHandler_->getGripPose(clay::InputHandlerXR::Hand::LEFT);
    ImGui::Text("Left Grip Pose: (%f, %f, %f, %f) (%f, %f, %f)",
               leftGripPose.orientation.x,
               leftGripPose.orientation.y,
               leftGripPose.orientation.z,
               leftGripPose.orientation.w,
               leftGripPose.position.x,
               leftGripPose.position.y,
               leftGripPose.position.z
    );
    const auto& rightGripPose = mpInputHandler_->getGripPose(clay::InputHandlerXR::Hand::LEFT);
    ImGui::Text("Right Grip Pose: (%f, %f, %f, %f) (%f, %f, %f)",
                rightGripPose.orientation.x,
                rightGripPose.orientation.y,
                rightGripPose.orientation.z,
                rightGripPose.orientation.w,
                rightGripPose.position.x,
                rightGripPose.position.y,
                rightGripPose.position.z
    );
    const auto& leftAimPose = mpInputHandler_->getAimPose(clay::InputHandlerXR::Hand::LEFT);
    ImGui::Text("Left Aim Pose: (%f, %f, %f, %f) (%f, %f, %f)",
                leftAimPose.orientation.x,
                leftAimPose.orientation.y,
                leftAimPose.orientation.z,
                leftAimPose.orientation.w,
                leftAimPose.position.x,
                leftAimPose.position.y,
                leftAimPose.position.z
    );
    const auto& rightAimPose = mpInputHandler_->getAimPose(clay::InputHandlerXR::Hand::LEFT);
    ImGui::Text("Right Aim Pose: (%f, %f, %f, %f) (%f, %f, %f)",
                rightAimPose.orientation.x,
                rightAimPose.orientation.y,
                rightAimPose.orientation.z,
                rightAimPose.orientation.w,
                rightAimPose.position.x,
                rightAimPose.position.y,
                rightAimPose.position.z
    );
    const auto& leftJoystickDir = mpInputHandler_->getJoystickDirection(clay::InputHandlerXR::Hand::LEFT);
    ImGui::Text("Joystick dir: (%f, %f) (%f, %f)",
               leftJoystickDir.x,
               leftJoystickDir.y,
               leftJoystickDir.x,
               leftJoystickDir.y
    );
    const auto& headPose = mpInputHandler_->getHeadPose();
    ImGui::Text("Headpose (%f, %f %f, %f) (%f %f %f)",
                headPose.orientation.x,
                headPose.orientation.y,
                headPose.orientation.z,
                headPose.orientation.w,
                headPose.position.x,
                headPose.position.y,
                headPose.position.z
    );
    if (ImGui::Button("Play sound")) {
        mpScene_->playSound();
    }
    if (ImGui::Button("Next Scene")) {
        mpScene_->getApp()->setScene(new SpaceScene(mpScene_->getApp()));
        mpScene_->setRemove(true);
    }

    ImGui::End();
}

void SandboxGUI::pointAt(glm::vec3 rayOrigin, glm::vec3 rayDir) {
    // Calculate the denominator
    float denominator = glm::dot(mNormal_, rayDir);

    // Check if the ray is parallel to the plane
    if (glm::abs(denominator) < 1e-6f) {
        // No intersection (ray is parallel)
        return;
    }

    // Calculate the numerator
    glm::vec3 planeToRay = mPosition_ - rayOrigin;
    float t = glm::dot(planeToRay, mNormal_) / denominator;

    // Check if the intersection is behind the ray origin
    if (t < 0.0f) {
        // No intersection (plane is behind the ray origin)
        return;
    }

    // Calculate the intersection point
    glm::vec3 intersectPoint = rayOrigin + t * rayDir;

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(mRotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mRotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), mScale_);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mPosition_);

    float planeMinX = -0.5f, planeMaxX = 0.5f; // Plane bounds in local space
    float planeMinY = -0.5f, planeMaxY = 0.5f;

    // Step 1: Transform to local space
    glm::vec4 localPoint = glm::inverse(translationMatrix * rotationMatrix * scaleMatrix) * glm::vec4(intersectPoint, 1.0f);
    // Step 2: Map to UV space
    float u = (localPoint.x - planeMinX) / (planeMaxX - planeMinX);
    // use localPoint.z vr v since the planes default face is +y (so vertical on the plane is z dir)
    float v = (localPoint.z - planeMinY) / (planeMaxY - planeMinY);

    // Step 3: Scale to texture resolution
    int pixelX = static_cast<int>(u * mTextureDim_.x);
    int pixelY = static_cast<int>(v * mTextureDim_.y);

    // Step 4: Clamp to texture bounds
    pixelX = glm::clamp(pixelX, 0, mTextureDim_.x - 1);
    pixelY = glm::clamp(pixelY, 0, mTextureDim_.y - 1);

    calMousePos = ImVec2(pixelX, pixelY);
}