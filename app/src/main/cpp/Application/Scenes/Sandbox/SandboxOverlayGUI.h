#pragma once
// third party
#include <glm/glm.hpp>
// project
#include <clay/gui/xr/ImGuiComponentXR.h>

// forward declare AppXR
namespace clay {
class AppXR;
}

class SandboxOverlayGUI : public clay::ImGuiComponentXR {
public:
    SandboxOverlayGUI();

    ~SandboxOverlayGUI();

    void setApp(clay::AppXR* theApp);

    void setPosition(const glm::vec2 position);

    void render(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight);
    void setMousePos(const ImVec2& mousePos);

private:
    void buildImGui(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight);

    glm::vec2 mPosition_{0,0};

    clay::AppXR* mApp_ = nullptr;

    ImVec2 mousePos = ImVec2( 700.0f +50.f,  735.0f + 15.f);
};
