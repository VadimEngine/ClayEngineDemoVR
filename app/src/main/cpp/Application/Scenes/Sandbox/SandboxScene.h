#pragma once
// standard lib
// third party
#include <glm/glm.hpp>
// clay
#include <clay/audio/Audio.h>
#include <clay/entity/Entity.h>
#include <clay/graphics/xr/GraphicsContextXR.h>
#include <clay/application/common/BaseScene.h>
#include <clay/application/xr/CameraControllerXR.h>
// project
#include "Application/Scenes/Sandbox/SandboxOverlayGUI.h"
#include "Application/Scenes/Sandbox/SandboxSceneGUI.h"

class SandboxScene : public clay::BaseScene {
public:
    SandboxScene(clay::IApp& parentApp);

    ~SandboxScene() = default;

    void update(float dt) override;

    void render(clay::IGraphicsContext& gContext) override;

    void renderGUI() override;

    void playSound();

    void assembleResources() override;

public:
    SandboxOverlayGUI testGUI;
    std::unique_ptr<SandboxSceneGUI> mSandboxGUI_;

    clay::Entity mLeftHandEntity_; // TODO make hand(s) class to encapsulate the hand logic
    clay::Entity mRightHandEntity_;
    clay::Entity mTexturedSphere_;
    clay::Entity mTextEntity_;
    clay::Entity mCenterSphere_;

    // Shader used to draw stencil highlight
    clay::ShaderProgram* mpSolidShader_ = nullptr;

    clay::Font* mpFont_ = nullptr;
    clay::Audio* mpBeepDeepAudio_ = nullptr;

    bool mHighLight = false;

    clay::CameraControllerXR mCameraController_;
};


