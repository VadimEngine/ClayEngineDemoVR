#pragma once
#include "VR/GraphicsOpenGLES.h"
#include "VR/SampleGUI.h"

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) vertexPositions[V1], vertexPositions[V2], vertexPositions[V3], vertexPositions[V4], vertexPositions[V5], vertexPositions[V6],

class VRApp {
public:
    struct AndroidAppState {
        ANativeWindow *nativeWindow = nullptr;
        bool resumed = false;
    };

    struct SwapchainInfo {
        XrSwapchain swapchain = XR_NULL_HANDLE;
        int64_t swapchainFormat = 0;
        std::vector<void *> imageViews;
    };

    struct RenderLayerInfo {
        XrTime predictedDisplayTime = 0;
        std::vector<XrCompositionLayerBaseHeader *> layers;
        XrCompositionLayerProjection layerProjection = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
        std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
    };

    struct CameraConstants {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1;
        XrVector4f pad2;
        XrVector4f pad3;
    };

    static AndroidAppState androidAppState;
    static android_app *androidApp;

    static void AndroidAppHandleCmd(struct android_app *app, int32_t cmd);

    VRApp() = default;
    ~VRApp() = default;

    void run();

    void createInstance();

    void createDebugMessenger();

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

    void renderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);

private:
    CameraConstants cameraConstants;
    XrVector4f normals[6] = {
            {1.00f, 0.00f, 0.00f, 0},
            {-1.00f, 0.00f, 0.00f, 0},
            {0.00f, 1.00f, 0.00f, 0},
            {0.00f, -1.00f, 0.00f, 0},
            {0.00f, 0.00f, 1.00f, 0},
            {0.00f, 0.0f, -1.00f, 0}};
    size_t renderCuboidIndex = 0;
    XrInstance m_xrInstance = XR_NULL_HANDLE;
    std::vector<const char *> m_activeAPILayers = {};
    std::vector<const char *> m_activeInstanceExtensions = {};
    XrFormFactor m_formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId m_systemID = {};
    XrSession m_session = {};
    bool m_applicationRunning = true;
    bool m_sessionRunning = false;
    std::vector<std::string> m_instanceExtensions = {};
    std::vector<std::string> m_apiLayers = {};
    XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding;
    XrSystemProperties m_systemProperties = {XR_TYPE_SYSTEM_PROPERTIES};
    XrSpace m_localSpace = XR_NULL_HANDLE;
    std::vector<SwapchainInfo> m_colorSwapchainInfos = {};
    std::vector<SwapchainInfo> m_depthSwapchainInfos = {};
    XrViewConfigurationType m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
    std::vector<XrViewConfigurationView> m_viewConfigurationViews;
    XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;
    XrEnvironmentBlendMode m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
    std::vector<XrEnvironmentBlendMode> m_environmentBlendModes = {};
    std::vector<XrEnvironmentBlendMode> m_applicationEnvironmentBlendModes = {XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE};
    std::vector<XrViewConfigurationType> m_viewConfigurations;
    std::vector<XrViewConfigurationType> m_applicationViewConfigurations = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
    std::unique_ptr<GraphicsOpenGLES> m_graphicsAPI = nullptr;
    // In STAGE space, viewHeightM should be 0. In LOCAL space, it should be offset downwards, below the viewer's initial position.
    float m_viewHeightM = 1.5f;
    // Vertex and index buffers: geometry for our cuboids.
    void *m_vertexBuffer = nullptr;
    void *m_indexBuffer = nullptr;
    // Camera values constant buffer for the shaders.
    void *m_uniformBuffer_Camera = nullptr;
    // The normals are stored in a uniform buffer to simplify our vertex geometry.
    void *m_uniformBuffer_Normals = nullptr;
    // We use only two shaders in this app.
    void *m_vertexShader = nullptr, *m_fragmentShader = nullptr;
    // The pipeline is a graphics-API specific state object.
    void *m_pipeline = nullptr;

    SampleGUI testGUI;
};