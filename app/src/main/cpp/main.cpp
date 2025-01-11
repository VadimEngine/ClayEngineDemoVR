// clay
#include <clay/application/xr/AppXR.h>
// project
#include "Application/Scenes/Sandbox/SandboxScene.h"

void android_main(struct android_app* androidApp) {
    JNIEnv* env;
    androidApp->activity->vm->AttachCurrentThread(&env, nullptr);

    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    OPENXR_CHECK(
        xrGetInstanceProcAddr(
            XR_NULL_HANDLE,
            "xrInitializeLoaderKHR",
            (PFN_xrVoidFunction*)&xrInitializeLoaderKHR
        ),
        "Failed to get InstanceProcAddr for xrInitializeLoaderKHR."
    )
    if (!xrInitializeLoaderKHR) {
        return;
    }

    // Fill out an XrLoaderInitInfoAndroidKHR structure and initialize the loader for Android.
    XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
    loaderInitializeInfoAndroid.applicationVM = androidApp->activity->vm;
    loaderInitializeInfoAndroid.applicationContext = androidApp->activity->clazz;
    OPENXR_CHECK(
        xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid),
        "Failed to initialize Loader for Android."
    )
    LOG_I("Starting Application");
    clay::AppXR mainApp(androidApp);
    mainApp.setScene(new SandboxScene(&mainApp));
    mainApp.run();
}