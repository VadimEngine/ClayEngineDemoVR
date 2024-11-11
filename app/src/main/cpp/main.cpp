// standard lib
#include <string>
// third party
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
// core
#include "Core/App.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <iostream>


App* myApp = nullptr;

void init(android_app* app) {
    myApp = new App(app->window);
    myApp->initializeOpenGL(app);
}

void shutdown() {
    if (myApp != nullptr) {
        myApp->shutdown();
    }
}

static void handleAppCmd(struct android_app* app, int32_t appCmd) {
    switch (appCmd) {
        case APP_CMD_INIT_WINDOW:
            init(app);
            break;
        case APP_CMD_TERM_WINDOW:
            shutdown();
            break;
        case APP_CMD_SAVE_STATE:
        case APP_CMD_GAINED_FOCUS:
        case APP_CMD_LOST_FOCUS:
        default:
            break;
    }
}

static int32_t handleInputEvent(struct android_app* app, AInputEvent* inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void android_main(struct android_app* app) {
    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

        XrInstance xrInstance = XR_NULL_HANDLE;

    // Define OpenXR application info
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "MyOpenXRApp", XR_MAX_APPLICATION_NAME_SIZE);
    appInfo.applicationVersion = 1;
    strncpy(appInfo.engineName, "NoEngine", XR_MAX_ENGINE_NAME_SIZE);
    appInfo.engineVersion = 1;
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    // Create the instance creation info structure
    XrInstanceCreateInfo createInfo = {};
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.next = nullptr;
    createInfo.createFlags = 0;
    createInfo.applicationInfo = appInfo;

    // Create the instance
    XrResult result = xrCreateInstance(&createInfo, &xrInstance);

    if (result == XR_SUCCESS) {
        std::cout << "OpenXR instance created successfully!" << std::endl;
    } else {
        std::cerr << "Failed to create OpenXR instance: " << result << std::endl;
    }

    while (true) {
        int out_events;
        struct android_poll_source* out_data;

        // Poll all events. If the app is not visible, this loop blocks until g_Initialized == true.
        while (ALooper_pollAll(myApp != nullptr && myApp->getG_Initialized() ? 0 : -1, nullptr, &out_events, (void**)&out_data) >= 0) {
            // Process one event
            if (out_data != nullptr) {
                out_data->process(app, out_data);
            }
            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0) {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (myApp != nullptr || !myApp->getG_Initialized()) {
                    shutdown();
                }
                return;
            }
        }
        myApp->render();
        myApp->update();
    }
}