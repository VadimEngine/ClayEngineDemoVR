#pragma once
// third party
#include <openxr/openxr.h>
#include <glm/vec4.hpp>
// project
#include "Core/Graphics/GraphicsContext.h"
#include "XR/Camera.h"

class XRApp;

class Scene {
public:
    Scene(XRApp* theApp);

    virtual ~Scene();

    virtual void update(float dt) = 0;

    virtual void render(GraphicsContext& gContext) = 0;

    /** If this scene is set for removal */
    bool isRemove() const;

    /** Set this Scene to be deleted */
    void setRunning(const bool running);

    /** If this scene is set for removal */
    bool isRunning() const;

    /**
     * Set the Scene background color
     * @param newColor background color
     */
    void setBackgroundColor(glm::vec4 newColor);

    /**
     * Get this Scene background color
     */
    glm::vec4 getBackgroundColor() const;

    void setRemove(const bool remove);

    XRApp* getApp();

protected:
    XRApp* mpApp_ = nullptr;
    VR::Camera mCamera_;
    glm::vec4 mBackgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f};
    /** If this scene should be deleted */
    bool mIsRemove_ = false;
    /** If this scene is currently updating */
    bool mIsRunning_ = true;
};

