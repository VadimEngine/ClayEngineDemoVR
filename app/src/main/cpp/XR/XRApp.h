#pragma once
// standard lib
#include <string>
// third party
#include <android/asset_manager.h>
#include <android/bitmap.h>
#include <android/imagedecoder.h>
// project
#include "XR/GraphicsOpenGLES.h" // include this on top
#include "Core/Application/Scene.h"
#include "Core/Audio/AudioManager.h"
#include "Core/GUI/ImguiComponent.h"
#include "Core/InputHandler.h"
#include "Core/Resource.h"
#include "XR/GraphicsContextXR.h"
#include "XR/XRUtils.h"

class XRApp {
public:
    struct AndroidAppState {
        ANativeWindow* nativeWindow = nullptr;
        bool resumed = false;
    };

    struct SwapchainInfo {
        XrSwapchain swapchain = XR_NULL_HANDLE;
        int64_t swapchainFormat = 0;
        std::vector<void*> imageViews;
    };

    struct RenderLayerInfo {
        XrTime predictedDisplayTime = 0;
        std::vector<XrCompositionLayerBaseHeader*> layers;
        XrCompositionLayerProjection layerProjection = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
        std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
    };

    static void AndroidAppHandleCmd(struct android_app* app, int32_t cmd);

    XRApp(android_app* pAndroidApp);

    ~XRApp();

    void run();

    void setScene(Scene* newScene);

    void quit();

    InputHandler& getInputHandler();

    AAssetManager* getAssetManager();

    Resource& getResources();

    AudioManager& getAudioManager();

private:
    void createInstance();

    void getInstanceProperties();

    void getSystemID();

    void getViewConfigurationViews();

    void getEnvironmentBlendModes();

    void createSession();

    void createReferenceSpace();

    void createSwapchains();

    void createResources();

    void pollSystemEvents();

    void pollEvents();

    void renderFrame();

    void destroySwapchains();

    void destroyReferenceSpace();

    void destroyResources();

    void destroySession();

    void destroyInstance();

    bool renderLayer(RenderLayerInfo &renderLayerInfo);

private:
    AndroidAppState mAndroidAppState_;
    android_app* mpAndroidApp_;
    XrInstance mXRInstance_ = XR_NULL_HANDLE;
    std::vector<const char*> mActiveAPILayers_ = {};
    std::vector<const char*> mActiveInstanceExtensions_ = {};
    XrFormFactor mFormFactor_ = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId mSystemID_ = {};
    XrSession mSession_ = {};
    bool mApplicationRunning_ = true;
    bool mSessionRunning_ = false;
    std::vector<std::string> mInstanceExtensions = {};
    std::vector<std::string> mApiLayers_ = {};
    XrSystemProperties mSystemProperties_ = {XR_TYPE_SYSTEM_PROPERTIES};
    XrSpace mLocalSpace_ = XR_NULL_HANDLE;
    XrSpace mHeadSpace_ = XR_NULL_HANDLE;
    std::vector<SwapchainInfo> mColorSwapchainInfos_ = {};
    std::vector<SwapchainInfo> mDepthSwapchainInfos_ = {};
    XrViewConfigurationType mViewConfiguration_ = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
    std::vector<XrViewConfigurationView> mViewConfigurationViews_;
    XrSessionState mSessionState_ = XR_SESSION_STATE_UNKNOWN;
    XrEnvironmentBlendMode mEnvironmentBlendMode_ = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
    std::vector<XrEnvironmentBlendMode> mEnvironmentBlendModes_ = {};
    std::vector<XrEnvironmentBlendMode> mApplicationEnvironmentBlendModes_ = {XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE};
    std::vector<XrViewConfigurationType> mViewConfigurations_;
    std::vector<XrViewConfigurationType> mApplicationViewConfigurations_ = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
    std::unique_ptr<GraphicsOpenGLES> mGraphicsAPI_ = nullptr;
    std::vector<Scene*> mScenes_;
    Resource mResources_;
    InputHandler mInputHandler_;
    AudioManager mAudioManger_;
};