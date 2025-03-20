#include <clay/application/xr/AppXR.h>
// class
#include "Application/Scenes/Farm/FarmScene.h"

FarmScene::FarmScene(clay::IApp& parentApp)
    : clay::BaseScene(parentApp),
      mSkyBoxEntity(*this),
      mLeftHandEntity_(*this),
      mRightHandEntity_(*this),
      mTreeEntity_(*this),
      mFloorEntity_(*this),
      mCameraController_(mpFocusCamera_) {
    assembleResources();
    mBackgroundColor_ = {0.0f, 0.0f, 1.0f, 1.0f};

    auto* pSimpleShader = mApp_.getResources().getResource<clay::ShaderProgram>("SimpleShader");
    auto* pFlatShader = mApp_.getResources().getResource<clay::ShaderProgram>("FlatShader");
    auto* pTextureShader =mApp_.getResources().getResource<clay::ShaderProgram>("TextureShader");
    auto* pTextureFlippedShader = mApp_.getResources().getResource<clay::ShaderProgram>("TextureFlipShader");

    mGUI_ = std::make_unique<FarmSceneGUI>(
        pTextureShader,
        mApp_.getResources().getResource<clay::Mesh>("Plane"),
        this
    );
    mGUI_->setPosition({2, 0, 0});
    mGUI_->setRotation({90, 0, 45});
    mGUI_->setInputHandler(&(( (clay::AppXR&)mApp_).getInputHandler()));

    {
        auto* cubeRenderable = mLeftHandEntity_.addRenderable<clay::ModelRenderable>();
        cubeRenderable->setShader(pFlatShader);
        cubeRenderable->setModel(mResources_.getResource<clay::Model>("GloveLeft"));
        cubeRenderable->setScale({0.2f, 0.2f, 0.2f});
        cubeRenderable->setColor(0xE0AC6900);
    }
    {
        auto* cubeRenderable = mRightHandEntity_.addRenderable<clay::ModelRenderable>();
        cubeRenderable->setShader(pFlatShader);
        cubeRenderable->setModel(mResources_.getResource<clay::Model>("GloveRight"));
        cubeRenderable->setScale({0.2f, 0.2f, 0.2f});
        cubeRenderable->setColor(0xE0AC6900);
    }
    {
        auto* renderableLeaves = mTreeEntity_.addRenderable<clay::ModelRenderable>();
        renderableLeaves->setModel(mResources_.getResource<clay::Model>("Sphere"));
        renderableLeaves->setShader(pFlatShader);
        renderableLeaves->setColor({0,1,0,1});
        renderableLeaves->setPosition({0,1,0});

        auto* renderableTrunk = mTreeEntity_.addRenderable<clay::ModelRenderable>();
        renderableTrunk->setModel(mResources_.getResource<clay::Model>("Cube"));
        renderableTrunk->setShader(pFlatShader);
        renderableTrunk->setColor(0x3D2412FF); // brown
        renderableTrunk->setScale({.1, 2, .1});

        mTreeEntity_.setPosition({0,0,0});
        mTreeEntity_.setScale({1,1,1});
    }
    {
        auto* renderable = mFloorEntity_.addRenderable<clay::ModelRenderable>();
        renderable->setModel(mResources_.getResource<clay::Model>("RectPlane"));
        renderable->setShader(pFlatShader);
        renderable->setColor({0,1,0,1});
        mFloorEntity_.setPosition({0,-1,0});
        mFloorEntity_.setOrientation(glm::angleAxis(glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f)));
        mFloorEntity_.setScale({50,50,50});
    }
    {
        auto* sphereRenderable = mSkyBoxEntity.addRenderable<clay::ModelRenderable>();
        sphereRenderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        sphereRenderable->setShader(pTextureFlippedShader);
        sphereRenderable->setTexture(
            0,
            mApp_.getResources().getResource<clay::Texture>("CloudSky")->getId(),
            "theTexture"
        );
        mSkyBoxEntity.setPosition({0,0,0});
        mSkyBoxEntity.setScale({1,1,1});
    }
}

void FarmScene::update(float dt) {
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
    mSkyBoxEntity.setOrientation(glm::conjugate(mpFocusCamera_->getOrientation()));

    const glm::vec3 cameraPosition = mpFocusCamera_->getPosition();
    const glm::quat cameraOrientation = mpFocusCamera_->getOrientation();

    // Rotate hands to match camera
    const glm::vec3 rotatedRight = cameraOrientation * rightHandPosition;
    const glm::vec3 rotatedLeft = cameraOrientation * leftHandPosition;

    rightHandPosition = cameraPosition + rotatedRight;
    leftHandPosition = cameraPosition  + rotatedLeft;
    const glm::vec3 rightHandForward = glm::normalize(rightHandOrientation * glm::conjugate(mpFocusCamera_->getOrientation()) * glm::vec3{0, 0, -1});

    {
        // update left hand
        mLeftHandEntity_.setOrientation(cameraOrientation * leftHandOrientation);
        mLeftHandEntity_.setPosition(leftHandPosition);
    }
    {
        // update right hand
        mRightHandEntity_.setOrientation(cameraOrientation * rightHandOrientation);
        mRightHandEntity_.setPosition(rightHandPosition);

        // point with right hand
        mGUI_->pointAt(
            rightHandPosition,
            glm::normalize(rightHandOrientation * cameraOrientation * glm::vec3{0, 0, -1})
        );
    }
}

void FarmScene::render(clay::IGraphicsContext& gContext) {
    auto& gContextVR = dynamic_cast<clay::GraphicsContextXR&>(gContext);
    {
        gContextVR.renderer.setCameraUBO(gContextVR.getRendererES().getCameraHeadLockedUBO());
        // draw skybox
        {
            gContextVR.graphicsAPI.enable(clay::IGraphicsAPI::Capability::CULL_FACE);
            gContextVR.graphicsAPI.cullFace(clay::IGraphicsAPI::PolygonModeFace::FRONT);
            gContextVR.graphicsAPI.depthMask(false);
            mSkyBoxEntity.render(gContext);
            gContextVR.graphicsAPI.depthMask(true);
            gContextVR.graphicsAPI.disable(clay::IGraphicsAPI::Capability::CULL_FACE);
        }
    }
    // Revert back to world lock render
    gContextVR.renderer.setCameraUBO(gContextVR.getRendererES().getCameraWorldLockedUBO());

    // First draw content without updating stencil
    gContextVR.graphicsAPI.stencilFunc(clay::IGraphicsAPI::TestFunction::ALWAYS, 0xFF);
    gContextVR.graphicsAPI.stencilMask(0x00); // Prevent writing to stencil buffer
    gContextVR.renderer.setCameraUBO(gContextVR.getRendererES().getCameraWorldLockedUBO());

    // draw hands
    mLeftHandEntity_.render(gContext);
    mRightHandEntity_.render(gContext);

    mTreeEntity_.render(gContext);
    mFloorEntity_.render(gContext);

    // GUI
    mGUI_->render(gContext);
}

void FarmScene::renderGUI() { }

void FarmScene::assembleResources() {
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