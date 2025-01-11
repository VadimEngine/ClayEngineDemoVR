#pragma once
#include "Application/Scenes/Space/SpaceGUI.h"
#include <clay/graphics/opengles/GraphicsContextXR.h>
#include <clay/graphics/common/Mesh.h>

class SpaceScene : public clay::SceneXR {
public:
    SpaceScene(clay::AppXR* theApp);

    ~SpaceScene() override;

    virtual void update(float dt) override;

    virtual void render(clay::IGraphicsContext& gContext) override;

private:
    clay::ShaderProgram* mpSimpleShader_ = nullptr;
    clay::ShaderProgram* mpTextShader_ = nullptr;
    clay::ShaderProgram* mpTextureShader_ = nullptr;

    clay::Mesh* mpSphereMesh_ = nullptr;
    clay::Mesh* mpPlaneMesh_ = nullptr;
    clay::Mesh* mpCubeMesh_ = nullptr;

    glm::vec3 mSunPosition_;
    glm::vec3 mPlanetPosition_;
    float mOrbitRadius_;

    // TODO add a plane gui
    SpaceGUI* spaceGUI;
};