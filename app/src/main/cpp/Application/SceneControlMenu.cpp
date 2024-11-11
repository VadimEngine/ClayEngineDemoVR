// Core
#include "Core/App.h"
// Application
#include "Application/SceneControlMenu.h"

SceneControlMenu::SceneControlMenu(App* pApp)
: mpApp_(pApp){};

SceneControlMenu::~SceneControlMenu() = default;

void SceneControlMenu::buildImGui() {
    static ImVec4 clear_color = ImVec4(0,0,0, 1.00f);
    ImGuiIO& io = ImGui::GetIO();
    static bool WantTextInputLast = false;

    if (io.WantTextInput && !WantTextInputLast)
        mpApp_->ShowSoftKeyboardInput();
    WantTextInputLast = io.WantTextInput;

    {
        static float f = 0.0f;
        static int counter = 0;
        static char s[128];

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::InputText("Text", s, sizeof(s));

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Move forward")) {
            mpApp_->getScene()->getCamera()->move({0,0,1}, -1);
        }
        if (ImGui::Button("Move back")) {
            mpApp_->getScene()->getCamera()->move({0,0,1}, 1);
        }

        ImGui::End();
    }
}
