#pragma once
// core
#include "Core/GUI/ImguiComponent.h"

//Forward declare app
class App;

class SceneControlMenu : public ImguiComponent {
public:

    App* mpApp_ = nullptr;

    SceneControlMenu(App* pApp);

    ~SceneControlMenu() override;

    void buildImGui() override;

};

