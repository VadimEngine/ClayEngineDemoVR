#pragma once
// third party
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <glm/glm.hpp>
// project
#include "Core/GUI/ImguiComponent.h"
#include "Core/Graphics/Shader.h"

class XRApp;

class SandboxOverlayGUI : public ImguiComponent {
public:
    SandboxOverlayGUI();

    ~SandboxOverlayGUI();

    void setApp(XRApp* theApp);

    void setPosition(const glm::vec2 position);


    void render(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight);
    void setMousePos(const ImVec2& mousePos);

private:
    void buildImGui(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight);

    glm::vec2 mPosition_{0,0};

    XRApp* mApp_ = nullptr;

    ImVec2 mousePos = ImVec2( 700.0f +50.f,  735.0f + 15.f);

};
