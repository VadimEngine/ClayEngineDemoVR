#include "XRApp.h"

static int32_t handleInputEvent(struct android_app* app, AInputEvent* inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void XRApp::AndroidAppHandleCmd(struct android_app* app, int32_t cmd) {
    auto* appState = (AndroidAppState*)app->userData;

    switch (cmd) {
        // There is no APP_CMD_CREATE. The ANativeActivity creates the application thread from onCreate().
        // The application thread then calls android_main().
        case APP_CMD_START: {
            break;
        }
        case APP_CMD_RESUME: {
            appState->resumed = true;
            break;
        }
        case APP_CMD_PAUSE: {
            appState->resumed = false;
            break;
        }
        case APP_CMD_STOP: {
            break;
        }
        case APP_CMD_DESTROY: {
            appState->nativeWindow = nullptr;
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            appState->nativeWindow = app->window;
            ImguiComponent::initialize(app->window);
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            appState->nativeWindow = nullptr;
            break;
        }
    }
}

XRApp::XRApp(android_app* pAndroidApp): mpAndroidApp_(pAndroidApp) {
    mpAndroidApp_->userData = &mAndroidAppState_;
    mpAndroidApp_->onAppCmd = XRApp::AndroidAppHandleCmd;
    mpAndroidApp_->onInputEvent = handleInputEvent;

    createInstance();
    getInstanceProperties();
    getSystemID();

    getViewConfigurationViews();
    getEnvironmentBlendModes();
    createSession();
    createReferenceSpace();
    createSwapchains();
    mInputHandler_.initialize(mXRInstance_, mSession_, mLocalSpace_, mHeadSpace_);
    createResources();
}

XRApp::~XRApp() {
    destroySwapchains();
    destroyReferenceSpace();
    destroyResources();
    destroySession();
    destroyInstance();
    ImguiComponent::deinitialize();
}


void XRApp::run() {
    while (mApplicationRunning_) {
        pollSystemEvents();
        pollEvents();
        if (mSessionRunning_) {
            renderFrame();
        }
    }
}

void XRApp::quit() {
    XrResult result = xrRequestExitSession(mSession_);
}

void XRApp::setScene(Scene* newScene) {
    //mScene_ = newScene;
    mScenes_.push_back(newScene);
}

InputHandler& XRApp::getInputHandler() {
    return mInputHandler_;
}

AAssetManager* XRApp::getAssetManager() {
    return mpAndroidApp_->activity->assetManager;
}

AudioManager& XRApp::getAudioManager() {
    return mAudioManger_;
}

Resource& XRApp::getResources() {
    return mResources_;
}

void XRApp::createInstance() {
    XrApplicationInfo AI;
    strncpy(AI.applicationName, "OpenXR Template", XR_MAX_APPLICATION_NAME_SIZE);
    AI.applicationVersion = 1;
    strncpy(AI.engineName, "OpenXR Engine", XR_MAX_ENGINE_NAME_SIZE);
    AI.engineVersion = 1;
    AI.apiVersion = XR_CURRENT_API_VERSION;
    mInstanceExtensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    mInstanceExtensions.emplace_back(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);

    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr);
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data());

    // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
    for (auto &requestLayer : mApiLayers_) {
        for (auto &layerProperty : apiLayerProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
                continue;
            } else {
                mActiveAPILayers_.push_back(requestLayer.c_str());
                break;
            }
        }
    }

    // Get all the Instance Extensions from the OpenXR instance.
    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data());

    // Check the requested Instance Extensions against the ones from the OpenXR runtime.
    // If an extension is found add it to Active Instance Extensions.
    // Log error if the Instance Extension is not found.
    for (auto &requestedInstanceExtension : mInstanceExtensions) {
        bool found = false;
        for (auto &extensionProperty : extensionProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
                continue;
            } else {
                mActiveInstanceExtensions_.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            LOG_E("Failed to find OpenXR instance extension");
        }
    }

    // Fill out an XrInstanceCreateInfo structure and create an XrInstance.
    XrInstanceCreateInfo instanceCI{XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = AI;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(mActiveAPILayers_.size());
    instanceCI.enabledApiLayerNames = mActiveAPILayers_.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(mActiveInstanceExtensions_.size());
    instanceCI.enabledExtensionNames = mActiveInstanceExtensions_.data();
    xrCreateInstance(&instanceCI, &mXRInstance_);
}

void XRApp::getInstanceProperties() {
    // Get the instance's properties and log the runtime name and version.
    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_CHECK(xrGetInstanceProperties(mXRInstance_, &instanceProperties), "Failed to get InstanceProperties.")
    LOG_I("OpenXR Runtime: %s - %d.%d.%d", instanceProperties.runtimeName, XR_VERSION_MAJOR(instanceProperties.runtimeVersion), XR_VERSION_MINOR(instanceProperties.runtimeVersion),  XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void XRApp::getSystemID() {
    // Get the XrSystemId from the instance and the supplied XrFormFactor.
    XrSystemGetInfo systemGI{XR_TYPE_SYSTEM_GET_INFO};
    systemGI.formFactor = mFormFactor_;
    xrGetSystem(mXRInstance_, &systemGI, &mSystemID_);

    // Get the System's properties for some general information about the hardware and the vendor.
    xrGetSystemProperties(mXRInstance_, mSystemID_, &mSystemProperties_);
}

void XRApp::getViewConfigurationViews() {
    // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(mXRInstance_, mSystemID_, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.")
    mViewConfigurations_.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(mXRInstance_, mSystemID_, viewConfigurationCount, &viewConfigurationCount, mViewConfigurations_.data()), "Failed to enumerate View Configurations.")

    // Pick the first application supported View Configuration Type con supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : mApplicationViewConfigurations_) {
        if (std::find(mViewConfigurations_.begin(), mViewConfigurations_.end(), viewConfiguration) != mViewConfigurations_.end()) {
            mViewConfiguration_ = viewConfiguration;
            break;
        }
    }
    if (mViewConfiguration_ == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
        LOG_E("Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO.");
        mViewConfiguration_ = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationViewCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(mXRInstance_, mSystemID_, mViewConfiguration_, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.")
    mViewConfigurationViews_.resize(viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_CHECK(
        xrEnumerateViewConfigurationViews(
            mXRInstance_,
            mSystemID_,
            mViewConfiguration_,
                viewConfigurationViewCount,
                &viewConfigurationViewCount,
            mViewConfigurationViews_.data()
        ),
        "Failed to enumerate ViewConfiguration Views."
    )
}

void XRApp::getEnvironmentBlendModes() {
    // Retrieves the available blend modes. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t environmentBlendModeCount = 0;
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(mXRInstance_, mSystemID_, mViewConfiguration_, 0, &environmentBlendModeCount, nullptr), "Failed to enumerate EnvironmentBlend Modes.")
    mEnvironmentBlendModes_.resize(environmentBlendModeCount);
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(mXRInstance_, mSystemID_, mViewConfiguration_, environmentBlendModeCount, &environmentBlendModeCount, mEnvironmentBlendModes_.data()), "Failed to enumerate EnvironmentBlend Modes.")

    // Pick the first application supported blend mode supported by the hardware.
    for (const XrEnvironmentBlendMode &environmentBlendMode : mApplicationEnvironmentBlendModes_) {
        if (std::find(mEnvironmentBlendModes_.begin(), mEnvironmentBlendModes_.end(), environmentBlendMode) != mEnvironmentBlendModes_.end()) {
            mEnvironmentBlendMode_ = environmentBlendMode;
            break;
        }
    }
    if (mEnvironmentBlendMode_ == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM) {
        LOG_W("Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.");
        mEnvironmentBlendMode_ = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    }
}

void XRApp::createSession() {
    // Create an XrSessionCreateInfo structure.
    XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};

    // Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
    // This call sets up a graphics API that's suitable for use with OpenXR.
    mGraphicsAPI_ = std::make_unique<GraphicsOpenGLES>(mXRInstance_, mSystemID_);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // or another suitable depth function

    sessionCI.next = mGraphicsAPI_->getGraphicsBinding();
    sessionCI.createFlags = 0;
    sessionCI.systemId = mSystemID_;

    OPENXR_CHECK(
        xrCreateSession(
            mXRInstance_,
            &sessionCI,
            &mSession_
        ),
        "Failed to create Session."
    )
}

void XRApp::createReferenceSpace() {
    // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
    XrReferenceSpaceCreateInfo referenceSpaceCI{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCI.poseInReferenceSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    xrCreateReferenceSpace(mSession_, &referenceSpaceCI, &mLocalSpace_);

    // Create the Head space (View space)
    XrReferenceSpaceCreateInfo headSpaceCI{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    headSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
    headSpaceCI.poseInReferenceSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    XrResult headSpaceResult = xrCreateReferenceSpace(mSession_, &headSpaceCI, &mHeadSpace_);
    if (XR_FAILED(headSpaceResult)) {
        LOG_E("Failed to create Head space: %d", headSpaceResult);
        return;
    }
}

void XRApp::createSwapchains() {
    // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
    uint32_t formatCount = 0;
    xrEnumerateSwapchainFormats(mSession_, 0, &formatCount, nullptr);
    std::vector<int64_t> formats(formatCount);
    xrEnumerateSwapchainFormats(mSession_, formatCount, &formatCount, formats.data());
    if (mGraphicsAPI_->selectDepthSwapchainFormat(formats) == 0) {
        LOG_E("Failed to find depth format for Swapchain.");
    }

    //Resize the SwapchainInfo to match the number of view in the View Configuration.
    mColorSwapchainInfos_.resize(mViewConfigurationViews_.size());
    mDepthSwapchainInfos_.resize(mViewConfigurationViews_.size());

    // Per view, create a color and depth swapchain, and their associated image views.
    for (size_t i = 0; i < mViewConfigurationViews_.size(); i++) {
        SwapchainInfo &colorSwapchainInfo = mColorSwapchainInfos_[i];
        SwapchainInfo &depthSwapchainInfo = mDepthSwapchainInfos_[i];

        // Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
        // Color.
        XrSwapchainCreateInfo swapchainCI{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.format = mGraphicsAPI_->selectColorSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = mViewConfigurationViews_[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = mViewConfigurationViews_[i].recommendedImageRectWidth;
        swapchainCI.height = mViewConfigurationViews_[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        xrCreateSwapchain(mSession_, &swapchainCI, &colorSwapchainInfo.swapchain);
        colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

        // Depth.
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        swapchainCI.format = mGraphicsAPI_->selectDepthSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = mViewConfigurationViews_[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = mViewConfigurationViews_[i].recommendedImageRectWidth;
        swapchainCI.height = mViewConfigurationViews_[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        xrCreateSwapchain(mSession_, &swapchainCI, &depthSwapchainInfo.swapchain);
        depthSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

        // Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
        uint32_t colorSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr), "Failed to enumerate Color Swapchain Images.")
        XrSwapchainImageBaseHeader* colorSwapchainImages = mGraphicsAPI_->allocateSwapchainImageData(colorSwapchainInfo.swapchain, GraphicsOpenGLES::SwapchainType::COLOR, colorSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages), "Failed to enumerate Color Swapchain Images.")

        uint32_t depthSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr), "Failed to enumerate Depth Swapchain Images.")
        XrSwapchainImageBaseHeader* depthSwapchainImages = mGraphicsAPI_->allocateSwapchainImageData(depthSwapchainInfo.swapchain, GraphicsOpenGLES::SwapchainType::DEPTH, depthSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages), "Failed to enumerate Depth Swapchain Images.")

        // Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
        for (uint32_t j = 0; j < colorSwapchainImageCount; j++) {
            GraphicsOpenGLES::ImageViewCreateInfo imageViewCI{};
            imageViewCI.image = mGraphicsAPI_->getSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCI.type = GraphicsOpenGLES::ImageViewCreateInfo::Type::RTV;
            imageViewCI.view = GraphicsOpenGLES::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCI.format = colorSwapchainInfo.swapchainFormat;
            imageViewCI.aspect = GraphicsOpenGLES::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(mGraphicsAPI_->createImageView(imageViewCI));
        }
        for (uint32_t j = 0; j < depthSwapchainImageCount; j++) {
            GraphicsOpenGLES::ImageViewCreateInfo imageViewCI{};
            imageViewCI.image = mGraphicsAPI_->getSwapchainImage(depthSwapchainInfo.swapchain, j);
            imageViewCI.type = GraphicsOpenGLES::ImageViewCreateInfo::Type::DSV;
            imageViewCI.view = GraphicsOpenGLES::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCI.format = depthSwapchainInfo.swapchainFormat;
            imageViewCI.aspect = GraphicsOpenGLES::ImageViewCreateInfo::Aspect::DEPTH_BIT;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(mGraphicsAPI_->createImageView(imageViewCI));
        }
    }
}

void XRApp::createResources() {
    // mResources_ add resources here
    auto* assetManager = getAssetManager();

    auto simpleVert = xr::utils::loadAsset(assetManager,"Shaders/SimpleVert.vert");
    auto simpleFrag = xr::utils::loadAsset(assetManager,"Shaders/SimpleFrag.frag");
    std::unique_ptr<Shader> simpleShader = std::make_unique<Shader>(
        std::string(simpleVert.begin(), simpleVert.end()).c_str(),
        std::string(simpleFrag.begin(), simpleFrag.end()).c_str()
    );

    auto textVert = xr::utils::loadAsset(assetManager,"Shaders/TextVert.vert");
    auto textFrag = xr::utils::loadAsset(assetManager,"Shaders/TextFrag.frag");
    std::unique_ptr<Shader> textShader = std::make_unique<Shader>(
        std::string(textVert.begin(), textVert.end()).c_str(),
        std::string(textFrag.begin(), textFrag.end()).c_str()
    );

    auto textureVert = xr::utils::loadAsset(assetManager,"Shaders/TextureVert.vert");
    auto textureFrag = xr::utils::loadAsset(assetManager,"Shaders/TextureFrag.frag");
    std::unique_ptr<Shader> textureShader = std::make_unique<Shader>(
        std::string(textureVert.begin(), textureVert.end()).c_str(),
        std::string(textureFrag.begin(), textureFrag.end()).c_str()
    );

    std::vector<Mesh> planeMeshList;
    Mesh::loadMeshes(assetManager,"Models/Plane.obj", planeMeshList);
    std::unique_ptr<Model> planeModel = std::make_unique<Model>();
    planeModel->addMeshes(std::move(planeMeshList));

    std::vector<Mesh> cubeMeshList;
    Mesh::loadMeshes(assetManager,"Models/Cube.obj", cubeMeshList);
    std::unique_ptr<Model> cubeModel = std::make_unique<Model>();
    cubeModel->addMeshes(std::move(cubeMeshList));

    std::vector<Mesh> sphereMeshList;
    Mesh::loadMeshes(assetManager,"Models/Sphere.obj", sphereMeshList);
    std::unique_ptr<Model> sphereModel = std::make_unique<Model>();
    sphereModel->addMeshes(std::move(sphereMeshList));

    std::unique_ptr<Audio> deepBeepAudio = std::make_unique<Audio>(
        Audio::loadAudio(assetManager, "Audio/beep_deep_1.wav")
    );

    std::unique_ptr<Font> consolasFont = std::make_unique<Font>(
        assetManager,"Fonts/Consolas.ttf"
    );

    // load texture
    unsigned char* pixels = nullptr;
    int width;
    int height;
    AndroidBitmapFormat format;

    xr::utils::loadImageAsset(
        assetManager,
        "Textures/V.png",
        reinterpret_cast<uint8_t**>(&pixels),
        width,
        height,
        format
    );

    std::unique_ptr<Texture> vTexture = std::make_unique<Texture>(pixels, width, height, 4);

    mResources_.addResource<Shader>(std::move(simpleShader), "SimpleShader");
    mResources_.addResource<Shader>(std::move(textShader), "TextShader");
    mResources_.addResource<Shader>(std::move(textureShader), "TextureShader");

    mResources_.addResource<Model>(std::move(planeModel), "Plane");
    mResources_.addResource<Model>(std::move(cubeModel), "Cube");
    mResources_.addResource<Model>(std::move(sphereModel), "Sphere");

    mResources_.addResource<Audio>(std::move(deepBeepAudio), "DeepBeep");

    mResources_.addResource<Font>(std::move(consolasFont), "Consolas");

    mResources_.addResource<Texture>(std::move(vTexture), "VTexture");
}

void XRApp::pollSystemEvents() {
    // Checks whether Android has requested that application should by destroyed.
    if (mpAndroidApp_->destroyRequested != 0) {
        mApplicationRunning_ = false;
        return;
    }
    while (true) {
        // Poll and process the Android OS system events.
        struct android_poll_source* source = nullptr;
        int events = 0;
        // The timeout depends on whether the application is active.
        const int timeoutMilliseconds = (!mAndroidAppState_.resumed && !mSessionRunning_ && mpAndroidApp_->destroyRequested == 0) ? -1 : 0;
        if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(mpAndroidApp_, source);
            }
        } else {
            break;
        }
    }
}

void XRApp::pollEvents() {
    XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
    auto XrPollEvents = [&]() -> bool {
        eventData = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(mXRInstance_, &eventData) == XR_SUCCESS;
    };

    while (XrPollEvents()) {
        switch (eventData.type) {
            // Log the number of lost events from the runtime.
            case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
                auto* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
                LOG_E("OPENXR: Events Lost: %d", eventsLost->lostEventCount);
                break;
            }
                // Log that an instance loss is pending and shutdown the application.
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
                auto* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
                LOG_E("OPENXR: Instance Loss Pending at: %lli", instanceLossPending->lossTime);
                mSessionRunning_ = false;
                mApplicationRunning_ = false;
                break;
            }
                // Log that the interaction profile has changed.
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
                auto* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
                LOG_E("OPENXR: Interaction Profile changed for Session: %llu", interactionProfileChanged->session);
                if (interactionProfileChanged->session != mSession_) {
                    LOG_E("XrEventDataInteractionProfileChanged for unknown Session");
                    break;
                }
                mInputHandler_.recordCurrentBindings();
                break;
            }
                // Log that there's a reference space change pending.
            case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
                auto* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
                LOG_E("OPENXR: Reference Space Change pending for Session: %llu",  referenceSpaceChangePending->session);

                if (referenceSpaceChangePending->session != mSession_) {
                    LOG_E("XrEventDataReferenceSpaceChangePending for unknown Session");
                    break;
                }
                break;
            }
                // Session State changes:
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                auto* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
                if (sessionStateChanged->session != mSession_) {
                    LOG_E("XrEventDataSessionStateChanged for unknown Session");
                    break;
                }

                if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
                    // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
                    XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                    sessionBeginInfo.primaryViewConfigurationType = mViewConfiguration_;
                    OPENXR_CHECK(xrBeginSession(mSession_, &sessionBeginInfo), "Failed to begin Session.")
                    mSessionRunning_ = true;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
                    // SessionState is stopping. End the XrSession.
                    OPENXR_CHECK(xrEndSession(mSession_), "Failed to end Session.")
                    mSessionRunning_ = false;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
                    // SessionState is exiting. Exit the application.
                    mSessionRunning_ = false;
                    mApplicationRunning_ = false;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
                    // SessionState is loss pending. Exit the application.
                    // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
                    mSessionRunning_ = false;
                    mApplicationRunning_ = false;
                }
                // Store state for reference across the application.
                mSessionState_ = sessionStateChanged->state;
                break;
            }
            default: {
                break;
            }
        }
    }
}

void XRApp::renderFrame() {
    // Get the XrFrameState for timing and rendering info.
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    OPENXR_CHECK(xrWaitFrame(mSession_, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.")

    // Tell the OpenXR compositor that the application is beginning the frame.
    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    OPENXR_CHECK(xrBeginFrame(mSession_, &frameBeginInfo), "Failed to begin the XR Frame.")

    // Variables for rendering and layer composition.
    RenderLayerInfo renderLayerInfo;
    renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

    // Check that the session is active and that we should render.
    bool sessionActive = (mSessionState_ == XR_SESSION_STATE_SYNCHRONIZED || mSessionState_ == XR_SESSION_STATE_VISIBLE || mSessionState_ == XR_SESSION_STATE_FOCUSED);
    if (sessionActive && frameState.shouldRender) {
        // pollActions(frameState.predictedDisplayTime);
        mInputHandler_.pollActions(frameState.predictedDisplayTime);
        // Render the stereo image and associate one of swapchain images with the XrCompositionLayerProjection structure.
        if (renderLayer(renderLayerInfo)) {
            renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.layerProjection));
        }
    }

    // Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = mEnvironmentBlendMode_;
    frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
    frameEndInfo.layers = renderLayerInfo.layers.data();
    OPENXR_CHECK(xrEndFrame(mSession_, &frameEndInfo), "Failed to end the XR Frame.")
}

void XRApp::destroySwapchains() {
    // Per view in the view configuration:
    for (size_t i = 0; i < mViewConfigurationViews_.size(); i++) {
        SwapchainInfo &colorSwapchainInfo = mColorSwapchainInfos_[i];
        SwapchainInfo &depthSwapchainInfo = mDepthSwapchainInfos_[i];

        // Destroy the color and depth image views from GraphicsAPI.
        for (void*& imageView : colorSwapchainInfo.imageViews) {
            mGraphicsAPI_->destroyImageView(imageView);
        }
        for (void*& imageView : depthSwapchainInfo.imageViews) {
            mGraphicsAPI_->destroyImageView(imageView);
        }

        // Free the Swapchain Image Data.
        mGraphicsAPI_->freeSwapchainImageData(colorSwapchainInfo.swapchain);
        mGraphicsAPI_->freeSwapchainImageData(depthSwapchainInfo.swapchain);

        // Destroy the swapchains.
        OPENXR_CHECK(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain")
        OPENXR_CHECK(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy Depth Swapchain")
    }
}

void XRApp::destroyReferenceSpace(){
    OPENXR_CHECK(xrDestroySpace(mLocalSpace_), "Failed to destroy Space.")
    OPENXR_CHECK(xrDestroySpace(mHeadSpace_), "Failed to destroy Space.")
}

void XRApp::destroyResources() {
    //delete mScene_;
    //mScene_ = nullptr;
}

void XRApp::destroySession(){
    OPENXR_CHECK(xrDestroySession(mSession_), "Failed to destroy Session.")
}

void XRApp::destroyInstance() {
    xrDestroyInstance(mXRInstance_);
}

bool XRApp::renderLayer(RenderLayerInfo &renderLayerInfo) {
    // Locate the views from the view configuration within the (reference) space at the display time.
    std::vector<XrView> views(mViewConfigurationViews_.size(), {XR_TYPE_VIEW});
    // Will contain information on whether the position and/or orientation is valid and/or tracked.
    XrViewState viewState{XR_TYPE_VIEW_STATE};
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewLocateInfo.viewConfigurationType = mViewConfiguration_;
    viewLocateInfo.displayTime = renderLayerInfo.predictedDisplayTime;
    viewLocateInfo.space = mLocalSpace_;
    uint32_t viewCount = 0;
    XrResult locateViewsResult = xrLocateViews(
        mSession_,
            &viewLocateInfo,
            &viewState,
            static_cast<uint32_t>(views.size()),
            &viewCount,
            views.data()
    );
    if (locateViewsResult != XR_SUCCESS) {
        LOG_E("Failed to locate Views.");
        return false;
    }

    // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
    renderLayerInfo.layerProjectionViews.resize(
        viewCount,
        {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}
    );

    // update scene before render TODO use real dt and not in a render method
    for (auto it = mScenes_.rbegin(); it != mScenes_.rend();) {
        if ((*it)->isRemove()) {
            // Erase and update the iterator
            it = decltype(it)(mScenes_.erase(std::next(it).base()));
        } else {
            (*it)->update(0);
            ++it;
        }
    }

    GraphicsContextXR gContext{};

    // Per view in the view configuration: (one for each eye)
    for (uint32_t i = 0; i < viewCount; i++) {
        SwapchainInfo &colorSwapchainInfo = mColorSwapchainInfos_[i];
        SwapchainInfo &depthSwapchainInfo = mDepthSwapchainInfos_[i];

        // Acquire and wait for an image from the swapchains.
        // Get the image index of an image in the swapchains.
        // The timeout is infinite.
        uint32_t colorImageIndex = 0;
        uint32_t depthImageIndex = 0;
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        OPENXR_CHECK(
            xrAcquireSwapchainImage(
                colorSwapchainInfo.swapchain,
                &acquireInfo,
                &colorImageIndex
                ),
                "Failed to acquire Image from the Color Swapchian"
        )
        OPENXR_CHECK(
            xrAcquireSwapchainImage(
                depthSwapchainInfo.swapchain,
                &acquireInfo,
                &depthImageIndex
            ),
            "Failed to acquire Image from the Depth Swapchian"
        )

        XrSwapchainImageWaitInfo waitInfo = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        OPENXR_CHECK(
            xrWaitSwapchainImage(
                colorSwapchainInfo.swapchain,
                &waitInfo
            ),
            "Failed to wait for Image from the Color Swapchain"
        )
        OPENXR_CHECK(
            xrWaitSwapchainImage(
                depthSwapchainInfo.swapchain,
                &waitInfo
            ),
            "Failed to wait for Image from the Depth Swapchain"
        )

        // Get the width and height and construct the viewport and scissors.
        const uint32_t &width = mViewConfigurationViews_[i].recommendedImageRectWidth;
        const uint32_t &height = mViewConfigurationViews_[i].recommendedImageRectHeight;
        GraphicsOpenGLES::Viewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
        GraphicsOpenGLES::Rect2D scissor = {{(int32_t)0, (int32_t)0}, {width, height}};

        // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
        // This also associates the swapchain image with this layer projection view.
        renderLayerInfo.layerProjectionViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        renderLayerInfo.layerProjectionViews[i].pose = views[i].pose;
        renderLayerInfo.layerProjectionViews[i].fov = views[i].fov;
        renderLayerInfo.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.x = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.y = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
        renderLayerInfo.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.

        // Rendering code to clear the color and depth image views.
        mGraphicsAPI_->beginRendering();

        if (mEnvironmentBlendMode_ == XR_ENVIRONMENT_BLEND_MODE_OPAQUE) {
            // VR mode use a background color.
            if (!mScenes_.empty()) {
                const auto bgColor = mScenes_.front()->getBackgroundColor();
                mGraphicsAPI_->clearColor(colorSwapchainInfo.imageViews[colorImageIndex], bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            } else {
                mGraphicsAPI_->clearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.1f, 0.1f, 0.1f, 1.0f);
            }
        } else {
            // In AR mode make the background color black.
            mGraphicsAPI_->clearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.0f, 0.0f, 0.0f, 1.0f);
        }
        mGraphicsAPI_->clearDepth(depthSwapchainInfo.imageViews[depthImageIndex], 1.0f);

        mGraphicsAPI_->setRenderAttachments(
                &colorSwapchainInfo.imageViews[colorImageIndex],
                1,
                depthSwapchainInfo.imageViews[depthImageIndex],
                width,
                height,
                nullptr //mScene_->m_pipeline
        );

        mGraphicsAPI_->setViewports(&viewport, 1);
        mGraphicsAPI_->setScissors(&scissor, 1);
        gContext.view = views[i];
        // for now, just render the first scene in the list
        if (!mScenes_.empty()) {
            mScenes_.front()->render(gContext);
        }
        mGraphicsAPI_->endRendering();

        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain")
        OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain")
    }

    // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
    renderLayerInfo.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.layerProjection.space = mLocalSpace_;
    renderLayerInfo.layerProjection.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.layerProjection.views = renderLayerInfo.layerProjectionViews.data();

    return true;
}
