#pragma once
// third party
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

class ImguiComponent {
public:
    ImguiComponent();

    virtual ~ImguiComponent() = default;

    static void initialize(ANativeWindow* pWindow);

    static void deinitialize();

    static void startRender();
    virtual void buildImGui();
    static void endRender();

    void render();
};


