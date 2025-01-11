#pragma once
// standard lib
// third party
#include <glm/glm.hpp>
// clay
#include <clay/application/xr/InputHandlerXR.h>
#include <clay/graphics/common/Mesh.h>
#include <clay/gui/xr/ImGuiComponentXR.h>
// project
#include "Application/Scenes/Common/XRUtils.h"

class SandboxScene;

class SandboxGUI : public clay::ImGuiComponentXR {
public:
    SandboxGUI(clay::ShaderProgram* textureShader, clay::Mesh* pPlaneMesh, SandboxScene* theScene);

    ~SandboxGUI();

    void render(const glm::mat4& view, const glm::mat4& proj);

    void setPosition(const glm::vec3& position);

    void setRotation(const glm::vec3& rotation);

    void setScale(const glm::vec3& scale);

    void pointAt(glm::vec3 origin, glm::vec3 direction);

    void setInputHandler(clay::InputHandlerXR* pInputHandler);

private:
    void buildImGui(const glm::mat4& view, const glm::mat4& proj);

    void renderPlane(const glm::mat4& view, const glm::mat4& proj);

    clay::ShaderProgram* mShader_ = nullptr;
    unsigned int mFBO_;
    unsigned int mFBTextureId_;

    glm::ivec2 mTextureDim_;

    glm::vec3 mPosition_{0,0,0};
    glm::vec3 mRotation_{0,0,0};
    glm::vec3 mScale_{1,1,1};
    glm::vec3 mNormal_ = {0,1,0};

    ImVec2 calMousePos = {0,0};

    clay::Mesh* mPlaneMesh_ = nullptr;

    clay::InputHandlerXR* mpInputHandler_ = nullptr;

    SandboxScene* mpScene_;
};


