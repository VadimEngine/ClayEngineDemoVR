//standard lib
// third party
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// project
#include "clay/utils/xr/UtilsXR.h"
#include "clay/application/xr/AppXR.h"
// class
#include "SandboxScene.h"

SandboxScene::SandboxScene(clay::IApp& parentApp)
    : clay::BaseScene(parentApp),
      mLeftHandEntity_(*this),
      mRightHandEntity_(*this),
      mTexturedSphere_(*this),
      mTextEntity_(*this),
      mCenterSphere_(*this),
      mCameraController_(mpFocusCamera_) {
    assembleResources();

    auto* pSimpleShader = mApp_.getResources().getResource<clay::ShaderProgram>("SimpleShader");
    auto* pFlatShader = mApp_.getResources().getResource<clay::ShaderProgram>("FlatShader");
    auto* pTextureShader =mApp_.getResources().getResource<clay::ShaderProgram>("TextureShader");
    auto* pTextureFlippedShader = mApp_.getResources().getResource<clay::ShaderProgram>("TextureFlipShader");
    mpSolidShader_ = mApp_.getResources().getResource<clay::ShaderProgram>("SolidShader");

    mpFont_ = mApp_.getResources().getResource<clay::Font>("Runescape");
    mpBeepDeepAudio_ = mApp_.getResources().getResource<clay::Audio>("DeepBeep");

    mSandboxGUI_ = std::make_unique<SandboxSceneGUI>(
        pTextureShader,
        mApp_.getResources().getResource<clay::Mesh>("Plane"),
        this
    );
    mSandboxGUI_->setPosition({2, 0, 0});
    mSandboxGUI_->setRotation({90, 0, 45});
    mSandboxGUI_->setInputHandler(&(( (clay::AppXR&)mApp_).getInputHandler()));

    testGUI.setApp(&(clay::AppXR&)mApp_);
    testGUI.setPosition({-.5,-.5});
    mBackgroundColor_ = {0.1f, 0.1f, 0.1f, 1.0f};

    {
        auto* handRenderable = mLeftHandEntity_.addRenderable<clay::ModelRenderable>();
        handRenderable->setShader(pFlatShader);
        handRenderable->setModel(mResources_.getResource<clay::Model>("GloveLeft"));
        handRenderable->setScale({0.2f, 0.2f, 0.2f});
        handRenderable->setColor(0xE0AC69FF);
    }
    {
        auto* handRenderable = mRightHandEntity_.addRenderable<clay::ModelRenderable>();
        handRenderable->setShader(pFlatShader);
        handRenderable->setModel(mResources_.getResource<clay::Model>("GloveRight"));
        handRenderable->setScale({0.2f, 0.2f, 0.2f});
        handRenderable->setColor(0xE0AC69FF);
    }
    {
        auto* renderable = mTexturedSphere_.addRenderable<clay::ModelRenderable>();
        renderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        renderable->setShader(pTextureFlippedShader);
        renderable->setTexture(0, mApp_.getResources().getResource<clay::Texture>("VTexture")->getId(), "theTexture");
        mTexturedSphere_.setPosition({-1,0,-2});
    }
    {
        auto* renderable = mCenterSphere_.addRenderable<clay::ModelRenderable>();
        renderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        renderable->setShader(pSimpleShader);
        renderable->setColor({1,1,1,1});
        mCenterSphere_.setPosition({0,0,0});
        mCenterSphere_.setScale({.1,.1,.1});
    }
    {
        auto* textRenderable = mTextEntity_.addRenderable<clay::TextRenderable>();
        textRenderable->setFont(mpFont_);
        textRenderable->setColor({1,1,0,1});
        textRenderable->setText("HELLO WORLD");
        mTextEntity_.setPosition({1,0,-2});
        mTextEntity_.setScale({.01f,.01f,.01f});
    }

    mApp_.getGraphicsAPI()->enable(clay::IGraphicsAPI::Capability::STENCIL_TEST);
    mApp_.getGraphicsAPI()->stencilFunc(clay::IGraphicsAPI::TestFunction::NOTEQUAL, 0xFF);
    mApp_.getGraphicsAPI()->stencilOp(
        clay::IGraphicsAPI::StencilAction::KEEP,
        clay::IGraphicsAPI::StencilAction::KEEP,
        clay::IGraphicsAPI::StencilAction::REPLACE
    );
}

void SandboxScene::update(float dt) {
    const auto joyDirLeft = ((clay::AppXR&)mApp_).getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::LEFT);
    const auto joyDirRight = ((clay::AppXR&)mApp_).getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::RIGHT);

    const auto& rightHandPose = ((clay::AppXR &) mApp_).getInputHandler().getAimPose(clay::InputHandlerXR::Hand::RIGHT);
    const glm::quat rightHandOrientation(rightHandPose.orientation.w, rightHandPose.orientation.x, rightHandPose.orientation.y, rightHandPose.orientation.z);
    glm::vec3 rightHandPosition = glm::vec3(rightHandPose.position.x,rightHandPose.position.y,rightHandPose.position.z);

    const auto& leftHandPose = ((clay::AppXR&)mApp_).getInputHandler().getAimPose(clay::InputHandlerXR::Hand::LEFT);
    const glm::quat leftHandOrientation(leftHandPose.orientation.w, leftHandPose.orientation.x, leftHandPose.orientation.y, leftHandPose.orientation.z);
    glm::vec3 leftHandPosition = glm::vec3(leftHandPose.position.x, leftHandPose.position.y, leftHandPose.position.z);
    const auto headPose = ((clay::AppXR&)mApp_).getInputHandler().getHeadPose();

    // update camera with input
    mCameraController_.updateWithJoystickInput(
        {joyDirLeft.x, joyDirLeft.y},
        {joyDirRight.x, joyDirRight.y},
        0.01f * 2.0f,
        1.0f/2.0f * 2.0f,
        headPose
    );
    const glm::vec3 cameraPosition = mpFocusCamera_->getPosition();
    const glm::quat cameraOrientation = mpFocusCamera_->getOrientation();

    // Rotate hands to match camera
    const glm::vec3 rotatedRight = cameraOrientation * rightHandPosition;
    const glm::vec3 rotatedLeft = cameraOrientation * leftHandPosition;

    rightHandPosition = cameraPosition + rotatedRight;
    leftHandPosition = cameraPosition  + rotatedLeft;

    {
        // update left hand
        mLeftHandEntity_.setOrientation(cameraOrientation * leftHandOrientation);
        mLeftHandEntity_.setPosition(leftHandPosition);
    }
    {
        // update right hand
        mRightHandEntity_.setOrientation(cameraOrientation * rightHandOrientation);
        mRightHandEntity_.setPosition(rightHandPosition);

        // highlight sphere if right hand is pointing at it
        const auto targetPosition = mTexturedSphere_.getPosition();
        const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), targetPosition);

        const glm::vec3 rightHandForward = glm::normalize(rightHandOrientation * cameraOrientation * glm::vec3{0, 0, -1});

        if (clay::utils::isRayIntersectingSphere(rightHandPosition, rightHandForward, targetPosition, 0.5)) {
            mHighLight = true;
        } else {
            mHighLight = false;
        }

        // point with right hand
        mSandboxGUI_->pointAt(
            rightHandPosition,
            rightHandForward
        );
    }
    // rotate the sphere and text
    mTexturedSphere_.getOrientation() *= glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mTextEntity_.getOrientation() *= glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void SandboxScene::render(clay::IGraphicsContext& gContext) {
    auto& gContextVR = dynamic_cast<clay::GraphicsContextXR&>(gContext);
    // First draw content without updating stencil
    gContextVR.graphicsAPI.stencilFunc(clay::IGraphicsAPI::TestFunction::ALWAYS, 0xFF);
    gContextVR.graphicsAPI.stencilMask(0x00); // Prevent writing to stencil buffer
    gContextVR.renderer.setCameraUBO(gContextVR.getRendererES().getCameraWorldLockedUBO());

    // draw hands
    mLeftHandEntity_.render(gContext);
    mRightHandEntity_.render(gContext);

    // Texture Sphere
    {
        if (mHighLight) {
            // draw content while updating stencil buffer
            gContextVR.graphicsAPI.stencilFunc(clay::IGraphicsAPI::TestFunction::ALWAYS, 0xFF);
            gContextVR.graphicsAPI.stencilMask(0xFF);
            mTexturedSphere_.render(gContext);

            // second pass
            {
                // draw content only where stencil has not been updated yet
                gContextVR.graphicsAPI.stencilFunc(clay::IGraphicsAPI::TestFunction::NOTEQUAL, 0xFF);
                gContextVR.graphicsAPI.stencilMask(0x00);
                // slightly scale up and set color for highlight
                mpSolidShader_->bind();
                mpSolidShader_->setMat4("uScaleMat", glm::scale(glm::mat4(1.0f), {1.1f, 1.1f, 1.1f}));
                mpSolidShader_->setVec4("uSolidColor", {1.0f, 1.0f, 0.0f, 1.0f});
                mTexturedSphere_.render(gContext, *mpSolidShader_);

                gContextVR.graphicsAPI.stencilFunc(clay::IGraphicsAPI::TestFunction::ALWAYS, 0xFF);
                gContextVR.graphicsAPI.stencilMask(0xFF);
            }
        } else {
            // draw content normally without using the stencil buffer
            mTexturedSphere_.render(gContext);
        }
    }
    // Draw sphere at local space origin
    mCenterSphere_.render(gContext);
    // Text
    mTextEntity_.render(gContext);
    // GUI
    mSandboxGUI_->render(gContext);
}

void SandboxScene::playSound() {
    mApp_.getAudioManager().playSound(mpBeepDeepAudio_->getId());
}

void SandboxScene::renderGUI() { }

void SandboxScene::assembleResources() {
    // Cube
    {
        std::unique_ptr<clay::Model> cubeModel = std::make_unique<clay::Model>();
        cubeModel->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("Cube")
        );
        mResources_.addResource(std::move(cubeModel), "Cube");
    }
    // Glove Left
    {
        std::unique_ptr<clay::Model> gloveModelLeft = std::make_unique<clay::Model>();
        gloveModelLeft->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("GloveLeft")
        );
        mResources_.addResource(std::move(gloveModelLeft), "GloveLeft");
    }
    // Glove Right
    {
        std::unique_ptr<clay::Model> gloveModelRight = std::make_unique<clay::Model>();
        gloveModelRight->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("GloveRight")
        );
        mResources_.addResource(std::move(gloveModelRight), "GloveRight");
    }
    // Plane
    {
        std::unique_ptr<clay::Model> planeModel = std::make_unique<clay::Model>();
        planeModel->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("RectPlane")
        );
        mResources_.addResource(std::move(planeModel), "RectPlane");
    }
    // Sphere
    {
        std::unique_ptr<clay::Model> sphereModel = std::make_unique<clay::Model>();
        sphereModel->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("Sphere")
        );
        mResources_.addResource(std::move(sphereModel), "Sphere");
    }
}