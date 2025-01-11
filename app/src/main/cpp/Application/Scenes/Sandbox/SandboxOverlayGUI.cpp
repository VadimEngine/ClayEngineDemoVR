// third party
#include <openxr/openxr.h>
// clay
#include <clay/application/xr/AppXR.h>
// class
#include "SandboxOverlayGUI.h"


SandboxOverlayGUI::SandboxOverlayGUI() {}

SandboxOverlayGUI::~SandboxOverlayGUI() {}

void SandboxOverlayGUI::setApp(clay::AppXR* theApp) {
    mApp_ = theApp;
}

void SandboxOverlayGUI::setPosition(const glm::vec2 position) {
    mPosition_ = position;
}

void SandboxOverlayGUI::render(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight) {
    startRender();
    buildImGui(view, proj, screenWidth, screenHeight);
    endRender();
}

void SandboxOverlayGUI::buildImGui(const glm::mat4& view, const glm::mat4& proj, uint32_t screenWidth, uint32_t screenHeight) {
    // Extract only the translation part of the view matrix (4th column) and ignore rotation
    auto viewTranslation = glm::mat4(1.0f);
    viewTranslation[3] = view[3];

    // Apply the translation to the world position
    glm::vec3 translatedPosition = glm::vec3(viewTranslation * glm::vec4(mPosition_.x, mPosition_.y, -2, 1.0f));

    // Transform the translated position into clip space
    glm::vec4 clipSpacePosition = proj * glm::vec4(translatedPosition, 1.0f);

    // Perform perspective division to get normalized device coordinates (NDC)
    if (clipSpacePosition.w != 0.0f) {
        clipSpacePosition /= clipSpacePosition.w;
    }

    // Map NDC to screen space (ImGui operates in screen coordinates)
    glm::vec2 screenPosition;
    screenPosition.x = (clipSpacePosition.x * 0.5f + 0.5f) * screenWidth;  // Map [-1, 1] to [0, screenSize.x]
    screenPosition.y = (1.0f - (clipSpacePosition.y * 0.5f + 0.5f)) * screenHeight;  // Map [-1, 1] to [0, screenSize.y] (Y inverted)

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = mousePos;
    io.AddMousePosEvent(io.MousePos.x, io.MousePos.y);  // Simulate a mouse position event
    io.AddMouseButtonEvent(0, true);  // Simulate mouse down
    io.AddMouseButtonEvent(0, false); // Simulate mouse up
    io.MouseDrawCursor = true;

    // Render ImGui at the computed screen position
    //ImGui::SetNextWindowPos(ImVec2(screenPosition.x, screenPosition.y), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(700,700), ImGuiCond_Always);

    ImGui::Begin("Hello, world!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    static int counter = 0;
    if (ImGui::Button("Move forward")) {
        ++counter;
    }

    ImGui::Text("Counter: %i", counter);

    if (ImGui::Button("Move back")) {}

    ImGui::End();
}

void SandboxOverlayGUI::setMousePos(const ImVec2& thisMousePos) {
    // mousePos = thisMousePos;
}
