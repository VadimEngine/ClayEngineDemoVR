#pragma once
// clay
#include <clay/graphics/xr/GraphicsContextXR.h>
#include <clay/application/xr/CameraControllerXR.h>
// project
#include "Application/Scenes/Space/SpaceSceneGUI.h"

class SpaceScene : public clay::BaseScene {
public:
    SpaceScene(clay::IApp& parentApp);

    ~SpaceScene() override;

    void update(float dt) override;

    void render(clay::IGraphicsContext& gContext) override;

    void renderGUI() override;

    void assembleResources() override;

    bool& getUpdateSpace();

private:
    clay::Entity mSkyBoxEntity;
    clay::Entity mLeftHandEntity_;
    clay::Entity mRightHandEntity_;

    clay::Entity mPlanetEntity_;
    clay::Entity mSunSphere_;
    clay::Entity mMoonEntity_;

    float planetOrbitSpeed = -1.0f/60.0f;
    float moonOrbitSpeed = -2.0f/60.0f;
    float mPlanetOrbitRadius_ = 2.0f;
    float mMoonOrbitRadius_ = .5f;

    bool mUpdateSpace_ = true;

    clay::CameraControllerXR mCameraController_;

    std::unique_ptr<SpaceSceneGUI> mSpaceGUI_;
};