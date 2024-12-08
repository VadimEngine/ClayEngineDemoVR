#pragma once
// standard lib
// third party
#include <glm/glm.hpp>
// project
#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Mesh.h"
#include "Core/Graphics/Model.h"
#include "Core/GUI/ImguiComponent.h"
#include "Core/InputHandler.h"
#include "XR/XRUtils.h"

class SpaceScene;

class SpaceGUI : public ImguiComponent {
public:
    SpaceGUI(Shader* textureShader, Model* pPlaneModel, SpaceScene* theScene);

    ~SpaceGUI();

    void render(const glm::mat4& view, const glm::mat4& proj);

    void setPosition(const glm::vec3& position);

    void setRotation(const glm::vec3& rotation);

    void setScale(const glm::vec3& scale);

    void pointAt(glm::vec3 origin, glm::vec3 direction);

    void setInputHandler(InputHandler* pInputHandler);
private:
    void buildImGui(const glm::mat4& view, const glm::mat4& proj);

    void renderPlane(const glm::mat4& view, const glm::mat4& proj);

    Shader* mShader_ = nullptr;
    GLuint mFBO_;
    GLuint mFBTextureId_;

    glm::ivec2 mTextureDim_;

    glm::vec3 mPosition_{0,0,0};
    glm::vec3 mRotation_{0,0,0};
    glm::vec3 mScale_{1,1,1};
    glm::vec3 mNormal_ = {0,1,0};

    ImVec2 calMousePos = {0,0};

    Model* mPlaneModel_ = nullptr;

    InputHandler* mpInputHandler_ = nullptr;

    SpaceScene* mpScene_;
};

