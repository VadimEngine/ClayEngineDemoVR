#pragma once
#include "Core/Application/Scene.h"
#include "Application/Scenes/Space/SpaceGUI.h"

class SpaceScene : public Scene {
public:
    SpaceScene(XRApp* theApp);

    ~SpaceScene() override;

    virtual void update(float dt) override;

    virtual void render(GraphicsContext& gContext) override;

private:
    Shader* mpSimpleShader_ = nullptr;
    Shader* mpTextShader_ = nullptr;
    Shader* mpTextureShader_ = nullptr;

    Model* mpSphereModel_ = nullptr;
    Model* mpPlaneModel_ = nullptr;
    Model* mpCubeModel_ = nullptr;

    glm::vec3 mSunPosition_;
    glm::vec3 mPlanetPosition_;
    float mOrbitRadius_;

    // TODO add a plane gui
    SpaceGUI* spaceGUI;
};