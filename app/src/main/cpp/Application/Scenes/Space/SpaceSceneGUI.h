#pragma once
// third party
#include <glm/glm.hpp>
// clay
#include <clay/graphics/common/Mesh.h>
#include <clay/gui/xr/ImGuiComponentXR.h>

class SpaceScene;

class SpaceSceneGUI : public clay::ImGuiComponentXR {
public:
    SpaceSceneGUI(clay::ShaderProgram* textureShader, clay::Mesh* pPlaneMesh, SpaceScene* theScene);

    ~SpaceSceneGUI();

    void render(clay::IGraphicsContext& gContext);

    void setPosition(const glm::vec3& position);

    void setRotation(const glm::vec3& rotation);

    void setScale(const glm::vec3& scale);

    void pointAt(glm::vec3 origin, glm::vec3 direction);

    void setInputHandler(clay::InputHandlerXR* pInputHandler);
private:
    void buildImGui(clay::IGraphicsContext& gContext);

    void renderPlane(clay::IGraphicsContext& gContext);

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

    SpaceScene* mpScene_;

    bool mSceneRunning_;

    unsigned int mSelectedSceneIdx = 0;
};

