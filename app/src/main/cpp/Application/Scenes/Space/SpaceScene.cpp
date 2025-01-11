// forward declare
#include <clay/application/xr/AppXR.h>
// class
#include "SpaceScene.h"


SpaceScene::SpaceScene(clay::AppXR* theApp)
 : clay::SceneXR(theApp) {
    mBackgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f};

    mpSimpleShader_ = mpApp_->getResources().getResource<clay::ShaderProgram>("SimpleShader");
    mpTextShader_ = mpApp_->getResources().getResource<clay::ShaderProgram>("TextShader");
    mpTextureShader_ = mpApp_->getResources().getResource<clay::ShaderProgram>("TextureShader");

    mpPlaneMesh_ = mpApp_->getResources().getResource<clay::Mesh>("Plane");
    mpSphereMesh_ = mpApp_->getResources().getResource<clay::Mesh>("Sphere");
    mpCubeMesh_ = mpApp_->getResources().getResource<clay::Mesh>("Cube");

    mOrbitRadius_ = 2;
    mSunPosition_ = {0.0f,0.0f,0.0f};
    mPlanetPosition_ = {0.0f,0.0f,mOrbitRadius_};

    spaceGUI = new SpaceGUI(mpTextureShader_, mpPlaneMesh_, this);
    spaceGUI->setPosition({1, 0, 4});
    spaceGUI->setRotation({90, 0, 45});
    spaceGUI->setInputHandler(&(mpApp_->getInputHandler()));
    mCamera_.setPosition({0,0,5});
}

SpaceScene::~SpaceScene() {}

void SpaceScene::update(float dt) {
    const auto joyDirLeft = mpApp_->getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::LEFT);
    const auto joyDirRight = mpApp_->getInputHandler().getJoystickDirection(clay::InputHandlerXR::Hand::RIGHT);

    mCamera_.updateWithJoystickInput(
        {joyDirLeft.x, joyDirLeft.y},
        {joyDirRight.x, joyDirRight.y},
        0.01f * 2.0f,
        1.0f/2.0f * 2.0f,
        mpApp_->getInputHandler().getHeadPose()
    );

    // update planet position
    float orbitRotationStep = glm::radians(2.0f);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), orbitRotationStep, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 rotatedVector = rotationMatrix * glm::vec4(mPlanetPosition_, 1.0f);
    mPlanetPosition_ = {rotatedVector.x, rotatedVector.y, rotatedVector.z};
};

void SpaceScene::render(clay::IGraphicsContext& gContext) {
    auto gContextVR = dynamic_cast<clay::GraphicsContextXR&>(gContext);
    mpSimpleShader_->bind();

    glm::mat4 glmView = xr::utils::computeViewMatrix(gContextVR.view.pose, mCamera_.getPosition(), mCamera_.getOrientation());
    glm::mat4 glmProj = xr::utils::computeProjectionMatrix(gContextVR.view.fov, mCamera_.nearZ, mCamera_.farZ);

    mpSimpleShader_->setMat4("view", glmView);
    mpSimpleShader_->setMat4("projection",  glmProj);

    // draw sun
    glm::vec3 sunScale(1,1,1);
    glm::mat4 sunScaleMat = glm::scale(glm::mat4(1.0f), sunScale);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mSunPosition_);
    mpSimpleShader_->setMat4("model", translationMatrix  * sunScaleMat);
    mpSimpleShader_->setVec4("uColor",  {1,1,0,1});
    mpSphereMesh_->render(*mpSimpleShader_);

    // draw planet
    glm::vec3 planetScale(.25f, .25f, .25f);
    glm::mat4 planetTranslateMax = glm::translate(glm::mat4(1.0f), mPlanetPosition_);
    glm::mat4 planetScaleMax = glm::scale(glm::mat4(1.0f), planetScale);
    mpSimpleShader_->setMat4("model", planetTranslateMax  * planetScaleMax);
    mpSimpleShader_->setVec4("uColor",  {0,1,0,1});
    mpSphereMesh_->render(*mpSimpleShader_);

    mpSimpleShader_->setVec4("uColor",  {1,1,0,1});
    // LEFT HAND
    {
        const auto& handPose = mpApp_->getInputHandler().getAimPose(clay::InputHandlerXR::Hand::LEFT);
        glm::quat handOrientation(handPose.orientation.w, handPose.orientation.x, handPose.orientation.y, handPose.orientation.z);
        glm::vec3 handScale(0.01f, 0.01f, 1.0f);

        // Calculate hand position relative to the camera's orientation
        glm::vec3 handPosition = glm::vec3(handPose.position.x, handPose.position.y, handPose.position.z);

        // Apply camera's orientation to hand position
        glm::mat4 rotationMatrix4 = glm::mat4_cast(handOrientation);
        glm::mat4 thisScaleMatrix = glm::scale(glm::mat4(1.0f), handScale);
        glm::mat4 thisTranslationMatrix = glm::translate(glm::mat4(1.0f), handPosition);

        // Update model matrix in shader
        glm::mat4 glmView2 = xr::utils::computeViewMatrix(gContextVR.view.pose);
        mpSimpleShader_->setMat4("view", glmView2);
        mpSimpleShader_->setMat4("model", thisTranslationMatrix * rotationMatrix4 * thisScaleMatrix);
        mpCubeMesh_->render(*mpSimpleShader_);
    }
    // RIGHT HAND
    {
        const auto& handPose = mpApp_->getInputHandler().getAimPose(clay::InputHandlerXR::Hand::RIGHT);
        glm::quat handOrientation(handPose.orientation.w, handPose.orientation.x, handPose.orientation.y, handPose.orientation.z);
        glm::vec3 handScale(0.01f, 0.01f, 1.0f);

        // Calculate hand position relative to the camera's orientation
        glm::vec3 handPosition = glm::vec3(handPose.position.x, handPose.position.y, handPose.position.z);

        // Apply camera's orientation to hand position
        glm::mat4 rotationMatrix4 = glm::mat4_cast(handOrientation);
        glm::mat4 thisScaleMatrix = glm::scale(glm::mat4(1.0f), handScale);
        glm::mat4 thisTranslationMatrix = glm::translate(glm::mat4(1.0f), handPosition);

        // Update model matrix in shader
        glm::mat4 glmView2 = xr::utils::computeViewMatrix(gContextVR.view.pose);
        mpSimpleShader_->setMat4("view", glmView2);
        mpSimpleShader_->setMat4("model", thisTranslationMatrix * rotationMatrix4 * thisScaleMatrix);
        mpCubeMesh_->render(*mpSimpleShader_);
        // point with right hand
        spaceGUI->pointAt(
            handPosition + mCamera_.getPosition(),
            glm::normalize(handOrientation * glm::conjugate(mCamera_.getOrientation()) * glm::vec3{0, 0, -1})
        );
    }

    spaceGUI->render(glmView, glmProj);
};