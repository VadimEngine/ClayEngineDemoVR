#pragma once
// standard lib
// third party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
// project
#include "XR/XRApp.h" // include this on top
#include "Application/Scenes/Sandbox/SandboxOverlayGUI.h"
#include "Application/Scenes/Sandbox/SandboxGUI.h"
#include "Core/Application/Scene.h"
#include "Core/Audio/Audio.h"
#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Mesh.h"
#include "Core/Graphics/Font.h"
#include "Core/Graphics/Model.h"
#include "XR/Camera.h"
#include "XR/GraphicsContextXR.h"
#include "XR/XRUtils.h"
#include "Core/Graphics/Texture.h"


class SandboxScene : public Scene {
public:
    SandboxScene(XRApp* pApp);

    ~SandboxScene() = default;

    void update(float dt) override;

    void render(GraphicsContext& gContext) override;

    void renderFont(const std::string& text,
                    const glm::mat4& view,
                    const glm::mat4& proj,
                    const glm::mat4& modelMat,
                    const Font& font,
                    const glm::vec3& scale,
                    const glm::vec3& color);

    void playSound();

public:
    SandboxOverlayGUI testGUI;
    SandboxGUI* mSandboxGUI_;

    Shader* mpSimpleShader_ = nullptr;
    Shader* mpTextShader_ = nullptr;
    Shader* mpTextureShader_ = nullptr;

    Model* mpPlaneModel_ = nullptr;
    Model* mpSphereModel_ = nullptr;
    Model* mpCubeModel_ = nullptr;

    Font* mpConsolasFont_ = nullptr;
    Audio* mpBeepDeepAudio_ = nullptr;

    Texture* mpVTexture_ = nullptr;
};


