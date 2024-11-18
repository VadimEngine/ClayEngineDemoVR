// core
#include "VR/VRApp.h"

android_app* VRApp::androidApp = nullptr;
VRApp::AndroidAppState VRApp::androidAppState = {};

static int32_t handleInputEvent(struct android_app* app, AInputEvent* inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void android_main(struct android_app* app) {
    JNIEnv* env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    XrInstance m_xrInstance = XR_NULL_HANDLE;  // Dummy XrInstance variable for OPENXR_CHECK macro.
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    OPENXR_CHECK(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR), "Failed to get InstanceProcAddr for xrInitializeLoaderKHR.");
    if (!xrInitializeLoaderKHR) {
        return;
    }

    // Fill out an XrLoaderInitInfoAndroidKHR structure and initialize the loader for Android.
    XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
    loaderInitializeInfoAndroid.applicationVM = app->activity->vm;
    loaderInitializeInfoAndroid.applicationContext = app->activity->clazz;
    OPENXR_CHECK(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid), "Failed to initialize Loader for Android.");

    app->userData = &VRApp::androidAppState;
    app->onAppCmd = VRApp::AndroidAppHandleCmd;
    app->onInputEvent = handleInputEvent;

    __android_log_print(ANDROID_LOG_INFO, "VRAPP", "%s", "STARTING APP");
    VRApp mainApp;
    VRApp::androidApp = app;
    mainApp.run();
}