// forward declare
#include <clay/application/xr/AppXR.h>
// class
#include "SpaceScene.h"

SpaceScene::SpaceScene(clay::IApp& parentApp)
    : clay::BaseScene(parentApp),
      mSkyBoxEntity(*this),
      mLeftHandEntity_(*this),
      mRightHandEntity_(*this),
      mPlanetEntity_(*this),
      mSunSphere_(*this),
      mMoonEntity_(*this),
      mCameraController_(mpFocusCamera_) {
    assembleResources();
    mBackgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f};

    auto* pSimpleShader = mApp_.getResources().getResource<clay::ShaderProgram>("SimpleShader");
    auto* pFlatShader = mApp_.getResources().getResource<clay::ShaderProgram>("FlatShader");
    auto* pTextureShader = mApp_.getResources().getResource<clay::ShaderProgram>("TextureShader");
    auto* pTextureFlippedShader = mApp_.getResources().getResource<clay::ShaderProgram>("TextureFlipShader");

    // update skybox texture to use linear filter
    mApp_.getGraphicsAPI()->bindTexture(
        clay::IGraphicsAPI::TextureTarget::TEXTURE_2D,
        mApp_.getResources().getResource<clay::Texture>("Galaxy")->getId()
    );
    mApp_.getGraphicsAPI()->texParameter(
        clay::IGraphicsAPI::TextureTarget::TEXTURE_2D,
        clay::IGraphicsAPI::TextureParameterType::TEXTURE_MIN_FILTER,
        clay::IGraphicsAPI::TextureParameterOption::LINEAR
    );
    mApp_.getGraphicsAPI()->texParameter(
        clay::IGraphicsAPI::TextureTarget::TEXTURE_2D,
        clay::IGraphicsAPI::TextureParameterType::TEXTURE_MAG_FILTER,
        clay::IGraphicsAPI::TextureParameterOption::LINEAR
    );
    mApp_.getGraphicsAPI()->bindTexture(
        clay::IGraphicsAPI::TextureTarget::TEXTURE_2D,
        0
    );

    mSpaceGUI_ = std::make_unique<SpaceSceneGUI>(
        pTextureShader,
        mApp_.getResources().getResource<clay::Mesh>("Plane"),
        this
    );
    mSpaceGUI_->setPosition({2, 0, 4});
    mSpaceGUI_->setRotation({90, 0, 45});
    mSpaceGUI_->setInputHandler(&((clay::AppXR&)mApp_).getInputHandler());
    mpFocusCamera_->setPosition({0,0,5});

    {
        auto* cubeRenderable = mLeftHandEntity_.addRenderable<clay::ModelRenderable>();
        cubeRenderable->setShader(pFlatShader);
        cubeRenderable->setModel(mResources_.getResource<clay::Model>("GloveLeft"));
        cubeRenderable->setScale({0.2f, 0.2f, 0.2f});
        cubeRenderable->setColor(0xE0AC69FF);
    }
    {
        auto* cubeRenderable = mRightHandEntity_.addRenderable<clay::ModelRenderable>();
        cubeRenderable->setShader(pFlatShader);
        cubeRenderable->setModel(mResources_.getResource<clay::Model>("GloveRight"));
        cubeRenderable->setScale({0.2f, 0.2f, 0.2f});
        cubeRenderable->setColor(0xE0AC69FF);
    }
    {
        auto* sphereRenderable = mPlanetEntity_.addRenderable<clay::ModelRenderable>();
        sphereRenderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        sphereRenderable->setShader(pTextureFlippedShader);
        sphereRenderable->setTexture(
            0,
            mApp_.getResources().getResource<clay::Texture>("Earth1k")->getId(),
            "theTexture"
        );
        mPlanetEntity_.setPosition({0.0f,0.0f,mPlanetOrbitRadius_});
        mPlanetEntity_.setScale({.30f, .30f, .30f});
    }
    {
        auto* sphereRenderable = mSunSphere_.addRenderable<clay::ModelRenderable>();
        sphereRenderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        sphereRenderable->setShader(pTextureFlippedShader);
        sphereRenderable->setTexture(
            0,
            mApp_.getResources().getResource<clay::Texture>("Sun")->getId(),
            "theTexture"
        );
        mSunSphere_.setPosition({0.0f,0.0f,0.0f});
    }
    {
        auto* sphereRenderable = mMoonEntity_.addRenderable<clay::ModelRenderable>();
        sphereRenderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        sphereRenderable->setShader(pTextureFlippedShader);
        sphereRenderable->setTexture(
            0,
            mApp_.getResources().getResource<clay::Texture>("Moon2k")->getId(),
            "theTexture"
        );
        const auto planetPostion = mPlanetEntity_.getPosition();
        mMoonEntity_.setPosition({planetPostion.x,planetPostion.y,planetPostion.z + mMoonOrbitRadius_});
        mMoonEntity_.setScale({.15f, .15f, .15f});
    }
    {
        auto* sphereRenderable = mSkyBoxEntity.addRenderable<clay::ModelRenderable>();
        sphereRenderable->setModel(mResources_.getResource<clay::Model>("Sphere"));
        sphereRenderable->setShader(pTextureFlippedShader);
        sphereRenderable->setTexture(
            0,
            mApp_.getResources().getResource<clay::Texture>("Galaxy")->getId(),
            "theTexture"
        );
        mSkyBoxEntity.setPosition({0,0,0});
        mSkyBoxEntity.setScale({1,1,1});
    }
}

SpaceScene::~SpaceScene() {}

void SpaceScene::update(float dt) {
    clay::AppXR& appXR = ((clay::AppXR&)mApp_);
    const auto joyDirLeft = appXR.getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::LEFT);
    const auto joyDirRight = appXR.getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::RIGHT);

    const auto& rightHandPose = appXR.getInputHandler().getAimPose(clay::InputHandlerXR::Hand::RIGHT);
    const glm::quat rightHandOrientation(rightHandPose.orientation.w, rightHandPose.orientation.x, rightHandPose.orientation.y, rightHandPose.orientation.z);
    glm::vec3 rightHandPosition = glm::vec3(rightHandPose.position.x,rightHandPose.position.y,rightHandPose.position.z);

    const auto& leftHandPose = appXR.getInputHandler().getAimPose(clay::InputHandlerXR::Hand::LEFT);
    const glm::quat leftHandOrientation(leftHandPose.orientation.w, leftHandPose.orientation.x, leftHandPose.orientation.y, leftHandPose.orientation.z);
    glm::vec3 leftHandPosition = glm::vec3(leftHandPose.position.x, leftHandPose.position.y, leftHandPose.position.z);

    const auto headPose = appXR.getInputHandler().getHeadPose();

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
    mSkyBoxEntity.setOrientation(glm::conjugate(mpFocusCamera_->getOrientation()));
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
        mSpaceGUI_->pointAt(
            rightHandPosition,
            glm::normalize(rightHandOrientation * cameraOrientation * glm::vec3{0, 0, -1})
        );
    }

    if (mUpdateSpace_) {
        glm::vec3 planetDiff;
        glm::vec3 planetPosition = mPlanetEntity_.getPosition();
        glm::vec3 moonPosition = mMoonEntity_.getPosition();
        {
            glm::vec3 orbitCenter = mSunSphere_.getPosition();
            // Update planet orbit
            glm::vec3 relativePos = planetPosition - orbitCenter;
            // Calculate angle of rotation based on current position
            float planetAngle = glm::atan(relativePos.z, relativePos.x);
            // Calculate distance from center
            float radius = glm::length(glm::vec2(relativePos.x, relativePos.z));
            // Update angle if planet is not at the center
            if (radius > 0.0f) {
                // Adjust the speed of rotation as needed
                planetAngle += planetOrbitSpeed;
            }

            // Calculate new position of the planet
            float x = orbitCenter.x + radius * glm::cos(planetAngle);
            float z = orbitCenter.z + radius * glm::sin(planetAngle);
            glm::vec3 newPlanetPos = glm::vec3(x, orbitCenter.y, z); // Y remains unchanged

            planetDiff = newPlanetPos - planetPosition;

            // Update planet position
            planetPosition = newPlanetPos;
        }
        {
            // Move moon along with the planet's movement
            moonPosition += planetDiff;

            // Orbit moon around planet
            glm::vec3 orbitCenter = planetPosition;
            glm::vec3 relativePos = moonPosition - orbitCenter;

            float moonAngle = glm::atan(relativePos.z, relativePos.x);
            float radius = glm::length(glm::vec2(relativePos.x, relativePos.z));

            // Update angle if moon is not at the center
            if (radius > 0.0f) {
                moonAngle += moonOrbitSpeed;
            }

            // Calculate new position of the moon
            float x = orbitCenter.x + radius * glm::cos(moonAngle);
            float z = orbitCenter.z + radius * glm::sin(moonAngle);
            glm::vec3 newMoonPos = glm::vec3(x, orbitCenter.y, z); // Y remains unchanged

            // Update moon position
            moonPosition = newMoonPos;
        }

        mPlanetEntity_.getOrientation() *= glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mPlanetEntity_.setPosition(planetPosition);

        mMoonEntity_.getOrientation() *= glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mMoonEntity_.setPosition(moonPosition);
    }
};

void SpaceScene::render(clay::IGraphicsContext& gContext) {
    auto gContextVR = dynamic_cast<clay::GraphicsContextXR&>(gContext);
    // Draw head locked content
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
    // render hands
    mLeftHandEntity_.render(gContext);
    mRightHandEntity_.render(gContext);

    mPlanetEntity_.render(gContext);
    mSunSphere_.render(gContext);
    mMoonEntity_.render(gContext);
    // draw gui
    mSpaceGUI_->render(gContext);
};

void SpaceScene::renderGUI() {}

bool& SpaceScene::getUpdateSpace() {
    return mUpdateSpace_;
}

void SpaceScene::assembleResources() {
    // Cube
    {
        std::unique_ptr<clay::Model> cubeModel = std::make_unique<clay::Model>();
        cubeModel->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("Cube")
        );
        mResources_.addResource(std::move(cubeModel), "Cube");
    }
    {
        // Glove Left
        std::unique_ptr<clay::Model> gloveModelLeft = std::make_unique<clay::Model>();
        gloveModelLeft->addSharedMesh(
            mApp_.getResources().getResource<clay::Mesh>("GloveLeft")
        );
        mResources_.addResource(std::move(gloveModelLeft), "GloveLeft");
    }
    {
        // Glove Right
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
