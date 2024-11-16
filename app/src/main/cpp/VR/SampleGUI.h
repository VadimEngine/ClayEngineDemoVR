#pragma once
#include "Core/GUI/ImguiComponent.h"

class SampleGUI : public ImguiComponent {
public:
    SampleGUI();

    ~SampleGUI();

    void buildImGui() override;
};
