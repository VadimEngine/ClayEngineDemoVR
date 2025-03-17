#pragma once
// third party
#include <glm/glm.hpp>
// clay
#include <clay/audio/Audio.h>
#include <clay/entity/Entity.h>
#include <clay/graphics/xr/GraphicsContextXR.h>
#include <clay/application/common/BaseScene.h>
#include <clay/application/xr/CameraControllerXR.h>
// project
#include "Application/Scenes/Farm/FarmSceneGUI.h"

class FarmScene : public clay::BaseScene {
public:
    FarmScene(clay::IApp& parentApp);

    ~FarmScene() = default;

    void update(float dt) override;

    void render(clay::IGraphicsContext& gContext) override;

    void renderGUI() override;

    void assembleResources() override;

private:
    clay::Entity mSkyBoxEntity;
    clay::Entity mLeftHandEntity_;
    clay::Entity mRightHandEntity_;

    clay::Entity mTreeEntity_;

    clay::Entity mFloorEntity_;

    clay::CameraControllerXR mCameraController_;

    std::unique_ptr<FarmSceneGUI> mGUI_;
};
