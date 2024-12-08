#pragma once
// standard lib
// third party
#include <openxr/openxr.h>
// project
#include "Core/Graphics/GraphicsContext.h"

class GraphicsContextXR : public GraphicsContext {
public:
    XrView view;
};

