#include "SampleGUI.h"

SampleGUI::SampleGUI() {

}

SampleGUI::~SampleGUI() {

}

void SampleGUI::buildImGui() {
    static ImVec4 clear_color = ImVec4(0,0,0, 1.00f);
    ImGuiIO& io = ImGui::GetIO();
    static bool WantTextInputLast = false;

    WantTextInputLast = io.WantTextInput;

    {
        static float f = 0.0f;
        static int counter = 0;
        static char s[128];
        ImGui::SetNextWindowPos(ImVec2(400, 400), ImGuiCond_FirstUseEver); // Set position

        ImGui::Begin("Hello, world!");

        ImGui::Text("This is some useful text.");

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color",(float*) &clear_color);

        if (ImGui::Button("Button"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::InputText("Text", s, sizeof(s));

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Move forward")) {}
        if (ImGui::Button("Move back")) {}

        ImGui::End();
    }
}
