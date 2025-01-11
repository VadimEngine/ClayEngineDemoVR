#pragma once
// standard lib
// third party
#include <glm/glm.hpp>
// clay
#include <clay/audio/Audio.h>
#include <clay/application/xr/AppXR.h>
#include <clay/graphics/common/Mesh.h>
#include <clay/graphics/common/Texture.h>
#include <clay/application/xr/SceneXR.h>
#include <clay/graphics/opengles/GraphicsContextXR.h>
// project
#include "Application/Scenes/Sandbox/SandboxOverlayGUI.h"
#include "Application/Scenes/Sandbox/SandboxGUI.h"


class SandboxScene : public clay::SceneXR {
public:
    SandboxScene(clay::AppXR* pApp);

    ~SandboxScene() = default;

    void update(float dt) override;

    void render(clay::IGraphicsContext& gContext) override;

    void renderFont(const std::string& text,
                    const glm::mat4& view,
                    const glm::mat4& proj,
                    const glm::mat4& modelMat,
                    const clay::Font& font,
                    const glm::vec3& scale,
                    const glm::vec3& color);

    void playSound();

public:
    SandboxOverlayGUI testGUI;
    SandboxGUI* mSandboxGUI_;

    clay::ShaderProgram* mpSimpleShader_ = nullptr;
    clay::ShaderProgram* mpTextShader_ = nullptr;
    clay::ShaderProgram* mpTextureShader_ = nullptr;

    clay::Mesh* mpPlaneMesh_ = nullptr;
    clay::Mesh* mpSphereMesh_ = nullptr;
    clay::Mesh* mpCubeMesh_ = nullptr;

    clay::Font* mpConsolasFont_ = nullptr;
    clay::Audio* mpBeepDeepAudio_ = nullptr;

    clay::Texture* mpVTexture_ = nullptr;
};


