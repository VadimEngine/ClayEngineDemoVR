#include "SandboxScene.h"

SandboxScene::SandboxScene(XRApp* pApp) : Scene(pApp){
    mpSimpleShader_ = mpApp_->getResources().getResource<Shader>("SimpleShader");
    mpTextShader_ = mpApp_->getResources().getResource<Shader>("TextShader");
    mpTextureShader_ = mpApp_->getResources().getResource<Shader>("TextureShader");

    mpPlaneModel_ = mpApp_->getResources().getResource<Model>("Plane");
    mpSphereModel_ = mpApp_->getResources().getResource<Model>("Sphere");
    mpCubeModel_ = mpApp_->getResources().getResource<Model>("Cube");

    mpConsolasFont_ = mpApp_->getResources().getResource<Font>("Consolas");
    mpBeepDeepAudio_ = mpApp_->getResources().getResource<Audio>("DeepBeep");

    mpVTexture_ = mpApp_->getResources().getResource<Texture>("VTexture");

    mSandboxGUI_ = new SandboxGUI(mpTextureShader_, mpPlaneModel_, this);
    mSandboxGUI_->setPosition({1, 0, 0});
    mSandboxGUI_->setRotation({90, 0, 45});
    mSandboxGUI_->setInputHandler(&(mpApp_->getInputHandler()));

    testGUI.setApp(pApp);
    testGUI.setPosition({-.5,-.5});
    mBackgroundColor_ = {0.1f, 0.1f, 0.1f, 1.0f};
}

void SandboxScene::update(float dt) {
    const auto joyDirLeft = mpApp_->getInputHandler().getJoystickDirection(InputHandler::Hand::LEFT);
    const auto joyDirRight = mpApp_->getInputHandler().getJoystickDirection(InputHandler::Hand::RIGHT);

    mCamera_.updateWithJoystickInput(
        {joyDirLeft.x, joyDirLeft.y},
        {joyDirRight.x, joyDirRight.y},
        0.01f * 2.0f,
        1.0f/2.0f * 2.0f,
        mpApp_->getInputHandler().getHeadPose()
    );
}

void SandboxScene::render(GraphicsContext& gContext) {
    auto gContextVR = dynamic_cast<GraphicsContextXR&>(gContext);
    glm::mat4 glmProj = xr::utils::computeProjectionMatrix(gContextVR.view.fov, mCamera_.nearZ, mCamera_.farZ);
    glm::mat4 glmView = xr::utils::computeViewMatrix(gContextVR.view.pose, mCamera_.getPosition(), mCamera_.getOrientation());

    mpSimpleShader_->bind();
    mpSimpleShader_->setMat4("projection",  glmProj);

    // Left hand
    {
        const auto& handPose = mpApp_->getInputHandler().getAimPose(InputHandler::Hand::LEFT);
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
        mpSimpleShader_->setVec4("uColor", {1.0f,1.0f,0.0f, 1.0f});
        mpCubeModel_->render(*mpSimpleShader_);
    }
    // Right hand
    {
        const auto& handPose = mpApp_->getInputHandler().getAimPose(InputHandler::Hand::RIGHT);
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
        mpSimpleShader_->setVec4("uColor", {1.0f,1.0f,0.0f, 1.0f});
        mpCubeModel_->render(*mpSimpleShader_);
        // point with right hand
        mSandboxGUI_->pointAt(
            handPosition + mCamera_.getPosition(),
            glm::normalize(handOrientation * glm::conjugate(mCamera_.getOrientation()) * glm::vec3{0, 0, -1})
        );
    }
    // Texture Sphere
    {
        static int count = 0;
        mpSimpleShader_->bind();

        glm::vec3 rotation(0,++count,0);
        glm::vec3 scale(1,1,1);
        glm::vec3 position(-1,0,-2);

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        mpSimpleShader_->setMat4("model", translationMatrix * rotationMatrix * scaleMatrix);

        // draw textured cube
        mpTextureShader_->bind();
        mpTextureShader_->setMat4("view", glmView);
        mpTextureShader_->setMat4("projection",  glmProj);
        mpTextureShader_->setMat4("model", translationMatrix * rotationMatrix * scaleMatrix);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mpVTexture_->getId());
        mpTextureShader_->setInt("theTexture", 0);

        mpSphereModel_->render(*mpTextureShader_);
    }

    // Font
    {
        static int count = 0;

        glm::vec3 rotation(0.0f,++count,0.0f);
        glm::vec3 scale(1,1,1);
        glm::vec3 position(1,0,-2);

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

        renderFont(
            "HELLO WORLD",
            glmView,
            glmProj,
            translationMatrix * rotationMatrix * scaleMatrix,
            *mpConsolasFont_,
            {.01f,.01f,.01f},
            {1,1,1}
        );
    }


    // GUI
    mSandboxGUI_->render(glmView, glmProj);
    // testGUI.render(glmView, glmProj, width, height);
}

void SandboxScene::playSound() {
    mpApp_->getAudioManager().playSound(mpBeepDeepAudio_->getId());
}

void SandboxScene::renderFont(const std::string& text,
                               const glm::mat4& view,
                               const glm::mat4& proj,
                               const glm::mat4& modelMat,
                               const Font& font,
                               const glm::vec3 &scale,
                               const glm::vec3 &color) {
    // activate corresponding render state
    mpTextShader_->bind();
    mpTextShader_->setVec3("textColor", color);
    mpTextShader_->setMat4("uView", view);
    mpTextShader_->setMat4("uProj", proj);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font.getVAO());

    // Calculate the total width of the text
    float totalWidth = 0.0f;
    for (const char& c : text) {
        const Font::Character* ch = font.getCharInfo(c);
        if (ch != nullptr) {
            totalWidth += (ch->advance >> 6) * scale.x;
        }
    }

    float startX = -totalWidth / 2.0f; // Center horizontally around the origin

    // iterate through all characters
    for (const char& c : text) {
        const Font::Character* ch = font.getCharInfo(c);

        if (ch != nullptr) {
            float xpos = startX + ch->bearing.x * scale.x;
            float ypos = - (ch->size.y - ch->bearing.y) * scale.y; // Adjust for Y-axis to center vertically around the origin

            float w = ch->size.x * scale.x;
            float h = ch->size.y * scale.y;
            // update VBO for each character
            float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }
            };
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch->textureId);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, font.getVBO());
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Apply the model matrix to the shader

            mpTextShader_->setMat4("uModel", modelMat);

            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            startX += (ch->advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
