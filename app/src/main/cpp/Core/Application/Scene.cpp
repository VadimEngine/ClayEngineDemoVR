// forward declare
#include "XR/XRApp.h"
// class
#include "Scene.h"

Scene::Scene(XRApp* theApp)
: mpApp_(theApp){}

Scene::~Scene() {}

/** If this scene is set for removal */
bool Scene::isRemove() const {
    return mIsRemove_;
}

/** Set this Scene to be deleted */
void Scene::setRunning(const bool running) {
    mIsRunning_ = running;
}

/** If this scene is set for removal */
bool Scene::isRunning() const {
    return mIsRunning_;
}

void Scene::setBackgroundColor(glm::vec4 newColor) {
    mBackgroundColor_ = newColor;
}

glm::vec4 Scene::getBackgroundColor() const {
    return mBackgroundColor_;
}

XRApp* Scene::getApp() {
    return mpApp_;
}

void Scene::setRemove(const bool remove) {
    mIsRemove_ = remove;
}


