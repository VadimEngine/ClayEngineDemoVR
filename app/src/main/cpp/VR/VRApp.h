#pragma once
#include "VR/GraphicsOpenGLES.h"
#include "VR/SampleGUI.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) vertexPositions[V1], vertexPositions[V2], vertexPositions[V3], vertexPositions[V4], vertexPositions[V5], vertexPositions[V6],

class VRApp {
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
    static android_app* androidApp;

    static void AndroidAppHandleCmd(struct android_app* app, int32_t cmd);

    VRApp() = default;
    ~VRApp() = default;

    std::string loadShaderFromAssets(const char* filePath);

    void run();

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

    void renderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);

    XrPath CreateXrPath(const char* path_string) {
        XrPath xrPath;
        OPENXR_CHECK(xrStringToPath(m_xrInstance, path_string, &xrPath), "Failed to create XrPath from string.");
        return xrPath;
    }
    std::string FromXrPath(XrPath path) {
        uint32_t strl;
        char text[XR_MAX_PATH_LENGTH];
        XrResult res;
        res = xrPathToString(m_xrInstance, path, XR_MAX_PATH_LENGTH, &strl, text);
        std::string str;
        if (res == XR_SUCCESS) {
            str = text;
        } else {
            OPENXR_CHECK(res, "Failed to retrieve path.");
        }
        return str;
    }
    void createActionSet() {
        XrActionSetCreateInfo actionSetCI{XR_TYPE_ACTION_SET_CREATE_INFO};
        // The internal name the runtime uses for this Action Set.
        strncpy(actionSetCI.actionSetName, "openxr-tutorial-actionset", XR_MAX_ACTION_SET_NAME_SIZE);
        // Localized names are required so there is a human-readable action name to show the user if they are rebinding Actions in an options screen.
        strncpy(actionSetCI.localizedActionSetName, "OpenXR Tutorial ActionSet", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
        // Set a priority: this comes into play when we have multiple Action Sets, and determines which Action takes priority in binding to a specific input.
        actionSetCI.priority = 0;

        OPENXR_CHECK(xrCreateActionSet(m_xrInstance, &actionSetCI, &m_actionSet), "Failed to create ActionSet.");

        auto CreateAction = [this](XrAction &xrAction, const char* name, XrActionType xrActionType, std::vector<const char*> subaction_paths = {}) -> void {
            XrActionCreateInfo actionCI{XR_TYPE_ACTION_CREATE_INFO};
            // The type of action: float input, pose, haptic output etc.
            actionCI.actionType = xrActionType;
            // Subaction paths, e.g. left and right hand. To distinguish the same action performed on different devices.
            std::vector<XrPath> subaction_xrpaths;
            for (auto p : subaction_paths) {
                subaction_xrpaths.push_back(CreateXrPath(p));
            }
            actionCI.countSubactionPaths = (uint32_t)subaction_xrpaths.size();
            actionCI.subactionPaths = subaction_xrpaths.data();
            // The internal name the runtime uses for this Action.
            strncpy(actionCI.actionName, name, XR_MAX_ACTION_NAME_SIZE);
            // Localized names are required so there is a human-readable action name to show the user if they are rebinding the Action in an options screen.
            strncpy(actionCI.localizedActionName, name, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
            OPENXR_CHECK(xrCreateAction(m_actionSet, &actionCI, &xrAction), "Failed to create Action.");
        };
        // An Action for grabbing cubes.
        CreateAction(m_grabCubeAction, "grab-cube", XR_ACTION_TYPE_FLOAT_INPUT, {"/user/hand/left", "/user/hand/right"});
        CreateAction(m_spawnCubeAction, "spawn-cube", XR_ACTION_TYPE_BOOLEAN_INPUT);
        CreateAction(m_changeColorAction, "change-color", XR_ACTION_TYPE_BOOLEAN_INPUT, {"/user/hand/left", "/user/hand/right"});
        // An Action for the position of the palm of the user's hand - appropriate for the location of a grabbing Actions.
        CreateAction(m_palmPoseAction, "palm-pose", XR_ACTION_TYPE_POSE_INPUT, {"/user/hand/left", "/user/hand/right"});
        // An Action for a vibration output on one or other hand.
        CreateAction(m_buzzAction, "buzz", XR_ACTION_TYPE_VIBRATION_OUTPUT, {"/user/hand/left", "/user/hand/right"});
        // For later convenience we create the XrPaths for the subaction path names.
        m_handPaths[0] = CreateXrPath("/user/hand/left");
        m_handPaths[1] = CreateXrPath("/user/hand/right");
    }

    void suggestBindings() {
        auto SuggestBindings = [this](const char* profile_path, std::vector<XrActionSuggestedBinding> bindings) -> bool {
            // The application can call xrSuggestInteractionProfileBindings once per interaction profile that it supports.
            XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
            interactionProfileSuggestedBinding.interactionProfile = CreateXrPath(profile_path);
            interactionProfileSuggestedBinding.suggestedBindings = bindings.data();
            interactionProfileSuggestedBinding.countSuggestedBindings = (uint32_t)bindings.size();
            if (xrSuggestInteractionProfileBindings(m_xrInstance, &interactionProfileSuggestedBinding) == XrResult::XR_SUCCESS)
                return true;
            //XR_TUT_LOG("Failed to suggest bindings with " << profile_path);
            return false;
        };
        bool any_ok = false;
        // Each Action here has two paths, one for each SubAction path.
        any_ok |= SuggestBindings("/interaction_profiles/khr/simple_controller", {{m_changeColorAction, CreateXrPath("/user/hand/left/input/select/click")},
                                                                                  {m_grabCubeAction, CreateXrPath("/user/hand/right/input/select/click")},
                                                                                  {m_spawnCubeAction, CreateXrPath("/user/hand/right/input/menu/click")},
                                                                                  {m_palmPoseAction, CreateXrPath("/user/hand/left/input/grip/pose")},
                                                                                  {m_palmPoseAction, CreateXrPath("/user/hand/right/input/grip/pose")},
                                                                                  {m_buzzAction, CreateXrPath("/user/hand/left/output/haptic")},
                                                                                  {m_buzzAction, CreateXrPath("/user/hand/right/output/haptic")}});
        // Each Action here has two paths, one for each SubAction path.
        any_ok |= SuggestBindings("/interaction_profiles/oculus/touch_controller", {{m_grabCubeAction, CreateXrPath("/user/hand/left/input/squeeze/value")},
                                                                                    {m_grabCubeAction, CreateXrPath("/user/hand/right/input/squeeze/value")},
                                                                                    {m_spawnCubeAction, CreateXrPath("/user/hand/right/input/a/click")},
                                                                                    {m_changeColorAction, CreateXrPath("/user/hand/left/input/trigger/value")},
                                                                                    {m_changeColorAction, CreateXrPath("/user/hand/right/input/trigger/value")},
                                                                                    {m_palmPoseAction, CreateXrPath("/user/hand/left/input/grip/pose")},
                                                                                    {m_palmPoseAction, CreateXrPath("/user/hand/right/input/grip/pose")},
                                                                                    {m_buzzAction, CreateXrPath("/user/hand/left/output/haptic")},
                                                                                    {m_buzzAction, CreateXrPath("/user/hand/right/output/haptic")}});
        if (!any_ok) {
            //DEBUG_BREAK;
        }
    }
    void RecordCurrentBindings() {
        if (m_session) {
            // now we are ready to:
            XrInteractionProfileState interactionProfile = {XR_TYPE_INTERACTION_PROFILE_STATE, 0, 0};
            // for each action, what is the binding?
            OPENXR_CHECK(xrGetCurrentInteractionProfile(m_session, m_handPaths[0], &interactionProfile), "Failed to get profile.");
            if (interactionProfile.interactionProfile) {
                //XR_TUT_LOG("user/hand/left ActiveProfile " << FromXrPath(interactionProfile.interactionProfile).c_str());
            }
            OPENXR_CHECK(xrGetCurrentInteractionProfile(m_session, m_handPaths[1], &interactionProfile), "Failed to get profile.");
            if (interactionProfile.interactionProfile) {
                //XR_TUT_LOG("user/hand/right ActiveProfile " << FromXrPath(interactionProfile.interactionProfile).c_str());
            }
        }
    }
    void createActionPoses() {
        // Create an xrSpace for a pose action.
        auto CreateActionPoseSpace = [this](XrSession session, XrAction xrAction, const char* subaction_path = nullptr) -> XrSpace {
            XrSpace xrSpace;
            const XrPosef xrPoseIdentity = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
            // Create frame of reference for a pose action
            XrActionSpaceCreateInfo actionSpaceCI{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            actionSpaceCI.action = xrAction;
            actionSpaceCI.poseInActionSpace = xrPoseIdentity;
            if (subaction_path)
                actionSpaceCI.subactionPath = CreateXrPath(subaction_path);
            OPENXR_CHECK(xrCreateActionSpace(session, &actionSpaceCI, &xrSpace), "Failed to create ActionSpace.");
            return xrSpace;
        };
        m_handPoseSpace[0] = CreateActionPoseSpace(m_session, m_palmPoseAction, "/user/hand/left");
        m_handPoseSpace[1] = CreateActionPoseSpace(m_session, m_palmPoseAction, "/user/hand/right");
    }
    void attachActionSet() {
        // Attach the action set we just made to the session. We could attach multiple action sets!
        XrSessionActionSetsAttachInfo actionSetAttachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        actionSetAttachInfo.countActionSets = 1;
        actionSetAttachInfo.actionSets = &m_actionSet;
        OPENXR_CHECK(xrAttachSessionActionSets(m_session, &actionSetAttachInfo), "Failed to attach ActionSet to Session.");
    }

    void pollActions(XrTime predictedTime) {
        // Update our action set with up-to-date input data.
        // First, we specify the actionSet we are polling.
        XrActiveActionSet activeActionSet{};
        activeActionSet.actionSet = m_actionSet;
        activeActionSet.subactionPath = XR_NULL_PATH;
        // Now we sync the Actions to make sure they have current data.
        XrActionsSyncInfo actionsSyncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
        actionsSyncInfo.countActiveActionSets = 1;
        actionsSyncInfo.activeActionSets = &activeActionSet;
        OPENXR_CHECK(xrSyncActions(m_session, &actionsSyncInfo), "Failed to sync Actions.");
        XrActionStateGetInfo actionStateGetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
        // We pose a single Action, twice - once for each subAction Path.
        actionStateGetInfo.action = m_palmPoseAction;
        // For each hand, get the pose state if possible.
        for (int i = 0; i < 2; i++) {
            // Specify the subAction Path.
            actionStateGetInfo.subactionPath = m_handPaths[i];
            OPENXR_CHECK(xrGetActionStatePose(m_session, &actionStateGetInfo, &m_handPoseState[i]), "Failed to get Pose State.");
            if (m_handPoseState[i].isActive) {
                XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
                XrResult res = xrLocateSpace(m_handPoseSpace[i], m_localSpace, predictedTime, &spaceLocation);
                if (XR_UNQUALIFIED_SUCCESS(res) &&
                    (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                    (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
                    m_handPose[i] = spaceLocation.pose;
                } else {
                    m_handPoseState[i].isActive = false;
                }
            }
        }
        for (int i = 0; i < 2; i++) {
            actionStateGetInfo.action = m_grabCubeAction;
            actionStateGetInfo.subactionPath = m_handPaths[i];
            OPENXR_CHECK(xrGetActionStateFloat(m_session, &actionStateGetInfo, &m_grabState[i]), "Failed to get Float State of Grab Cube action.");
        }
        for (int i = 0; i < 2; i++) {
            actionStateGetInfo.action = m_changeColorAction;
            actionStateGetInfo.subactionPath = m_handPaths[i];
            OPENXR_CHECK(xrGetActionStateBoolean(m_session, &actionStateGetInfo, &m_changeColorState[i]), "Failed to get Boolean State of Change Color action.");
        }
        // The Spawn Cube action has no subActionPath:
        {
            actionStateGetInfo.action = m_spawnCubeAction;
            actionStateGetInfo.subactionPath = 0;
            OPENXR_CHECK(xrGetActionStateBoolean(m_session, &actionStateGetInfo, &m_spawnCubeState), "Failed to get Boolean State of Spawn Cube action.");
        }

        if (m_grabState[0].isActive && m_grabState[0].currentState > 0.5 || m_grabState[1].isActive && m_grabState[1].currentState > 0.5) {
            grabFlag = !grabFlag;
        }

        for (int i = 0; i < 2; i++) {
            m_buzz[i] *= 0.5f;
            if (m_buzz[i] < 0.01f)
                m_buzz[i] = 0.0f;
            XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
            vibration.amplitude = m_buzz[i];
            vibration.duration = XR_MIN_HAPTIC_DURATION;
            vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

            XrHapticActionInfo hapticActionInfo{XR_TYPE_HAPTIC_ACTION_INFO};
            hapticActionInfo.action = m_buzzAction;
            hapticActionInfo.subactionPath = m_handPaths[i];
            OPENXR_CHECK(xrApplyHapticFeedback(m_session, &hapticActionInfo, (XrHapticBaseHeader*)&vibration), "Failed to apply haptic feedback.");
        }
    }

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
    std::vector<const char*> m_activeAPILayers = {};
    std::vector<const char*> m_activeInstanceExtensions = {};
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
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    // Camera values constant buffer for the shaders.
    void* m_uniformBuffer_Camera = nullptr;
    // The normals are stored in a uniform buffer to simplify our vertex geometry.
    void* m_uniformBuffer_Normals = nullptr;
    // We use only two shaders in this app.
    void* m_vertexShader = nullptr, *m_fragmentShader = nullptr;
    void* m_vertexShader2 = nullptr, *m_fragmentShader2 = nullptr;
    // The pipeline is a graphics-API specific state object.
    void* m_pipeline = nullptr;

    void* m_pipeline2 = nullptr;

    SampleGUI testGUI;

    XrActionSet m_actionSet;
    // An action for grabbing blocks, and an action to change the color of a block.
    XrAction m_grabCubeAction, m_spawnCubeAction, m_changeColorAction, m_buttonAction;
    // The realtime states of these actions.
    XrActionStateFloat m_grabState[2] = {{XR_TYPE_ACTION_STATE_FLOAT}, {XR_TYPE_ACTION_STATE_FLOAT}};
    XrActionStateBoolean m_changeColorState[2] = {{XR_TYPE_ACTION_STATE_BOOLEAN}, {XR_TYPE_ACTION_STATE_BOOLEAN}};
    XrActionStateBoolean m_spawnCubeState = {XR_TYPE_ACTION_STATE_BOOLEAN};
    // The haptic output action for grabbing cubes.
    XrAction m_buzzAction;
    // The current haptic output value for each controller.
    float m_buzz[2] = {0, 0};
    // The action for getting the hand or controller position and orientation.
    XrAction m_palmPoseAction;
    // The XrPaths for left and right hand hands or controllers.
    XrPath m_handPaths[2] = {0, 0};
    // The spaces that represents the two hand poses.
    XrSpace m_handPoseSpace[2];
    XrActionStatePose m_handPoseState[2] = {{XR_TYPE_ACTION_STATE_POSE}, {XR_TYPE_ACTION_STATE_POSE}};
    // The current poses obtained from the XrSpaces.
    XrPosef m_handPose[2] = {
            {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -m_viewHeightM}},
            {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -m_viewHeightM}}};

    float clearColor[4] = {0,0,0,1};

    bool grabFlag = false;
};