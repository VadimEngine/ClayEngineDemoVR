#pragma once
// clay
#include <clay/application/xr/AppXR.h>

struct SceneDetail {
    std::string mName_;
    std::string mDetails_;
    unsigned int mPreviewTextureId_;
    std::function<void()> mLoadScene_;
};

class DemoAppXR : public clay::AppXR {
public:
    DemoAppXR(android_app* pAndroidApp);

    void initialize() override;

    void createResources() override;

    std::vector<SceneDetail>& getSceneDetails();
private:
    std::vector<SceneDetail> mSceneDetails_;
};

