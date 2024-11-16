#include "VRApp.h"

void VRApp::AndroidAppHandleCmd(struct android_app *app, int32_t cmd) {
    AndroidAppState *appState = (AndroidAppState*)app->userData;

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

void VRApp::run() {
    createInstance();
    createDebugMessenger();
    getInstanceProperties();
    getSystemID();
    getViewConfigurationViews();
    getEnvironmentBlendModes();
    createSession();
    createReferenceSpace();
    createSwapchains();
    createResources();

    while (m_applicationRunning) {
        pollSystemEvents();
        pollEvents();
        if (m_sessionRunning) {
            renderFrame();
        }
    }

    destroySwapchains();
    destroyReferenceSpace();
    destroyResources();
    destroySession();
    destroyInstance();
}

void VRApp::createInstance() {
    XrApplicationInfo AI;
    strncpy(AI.applicationName, "OpenXR Tutorial Chapter 3", XR_MAX_APPLICATION_NAME_SIZE);
    AI.applicationVersion = 1;
    strncpy(AI.engineName, "OpenXR Engine", XR_MAX_ENGINE_NAME_SIZE);
    AI.engineVersion = 1;
    AI.apiVersion = XR_CURRENT_API_VERSION;
    m_instanceExtensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_instanceExtensions.emplace_back(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);

    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr);
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data());

    // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
    for (auto &requestLayer : m_apiLayers) {
        for (auto &layerProperty : apiLayerProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
                continue;
            } else {
                m_activeAPILayers.push_back(requestLayer.c_str());
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
    for (auto &requestedInstanceExtension : m_instanceExtensions) {
        bool found = false;
        for (auto &extensionProperty : extensionProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
                continue;
            } else {
                m_activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "Failed to find OpenXR instance extension" );
        }
    }

    // Fill out an XrInstanceCreateInfo structure and create an XrInstance.
    XrInstanceCreateInfo instanceCI{XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = AI;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
    instanceCI.enabledApiLayerNames = m_activeAPILayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
    instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();
    xrCreateInstance(&instanceCI, &m_xrInstance);
}

void VRApp::createDebugMessenger() {}

void VRApp::getInstanceProperties() {
    // Get the instance's properties and log the runtime name and version.
    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");
    __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "OpenXR Runtime: " );

//         std::cout << "OpenXR Runtime: " << instanceProperties.runtimeName << " - "
//                                       << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
//                                       << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
//                                       << XR_VERSION_PATCH(instanceProperties.runtimeVersion) << std::endl;
}

void VRApp::getSystemID() {
    // Get the XrSystemId from the instance and the supplied XrFormFactor.
    XrSystemGetInfo systemGI{XR_TYPE_SYSTEM_GET_INFO};
    systemGI.formFactor = m_formFactor;
    xrGetSystem(m_xrInstance, &systemGI, &m_systemID);

    // Get the System's properties for some general information about the hardware and the vendor.
    xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties);
}

void VRApp::getViewConfigurationViews() {
    // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
    m_viewConfigurations.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

    // Pick the first application supported View Configuration Type con supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : m_applicationViewConfigurations) {
        if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end()) {
            m_viewConfiguration = viewConfiguration;
            break;
        }
    }
    if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
        std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
        m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationViewCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
    m_viewConfigurationViews.resize(viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");
}

void VRApp::getEnvironmentBlendModes() {
    // Retrieves the available blend modes. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t environmentBlendModeCount = 0;
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, 0, &environmentBlendModeCount, nullptr), "Failed to enumerate EnvironmentBlend Modes.");
    m_environmentBlendModes.resize(environmentBlendModeCount);
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, environmentBlendModeCount, &environmentBlendModeCount, m_environmentBlendModes.data()), "Failed to enumerate EnvironmentBlend Modes.");

    // Pick the first application supported blend mode supported by the hardware.
    for (const XrEnvironmentBlendMode &environmentBlendMode : m_applicationEnvironmentBlendModes) {
        if (std::find(m_environmentBlendModes.begin(), m_environmentBlendModes.end(), environmentBlendMode) != m_environmentBlendModes.end()) {
            m_environmentBlendMode = environmentBlendMode;
            break;
        }
    }
    if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE." );
        m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    }
}

void VRApp::createSession() {
    // Create an XrSessionCreateInfo structure.
    XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};

    // Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
    // This call sets up a graphics API that's suitable for use with OpenXR.
    m_graphicsAPI = std::make_unique<GraphicsOpenGLES>(m_xrInstance, m_systemID); // crash here
    sessionCI.next = m_graphicsAPI->getGraphicsBinding();
    sessionCI.createFlags = 0;
    sessionCI.systemId = m_systemID;

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");
}

void VRApp::createReferenceSpace() {
    // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
    XrReferenceSpaceCreateInfo referenceSpaceCI{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCI.poseInReferenceSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_localSpace);
}

void VRApp::createSwapchains() {
    // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
    uint32_t formatCount = 0;
    xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr);
    std::vector<int64_t> formats(formatCount);
    xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data());
    if (m_graphicsAPI->selectDepthSwapchainFormat(formats) == 0) {
        std::cerr << "Failed to find depth format for Swapchain." << std::endl;
    }

    //Resize the SwapchainInfo to match the number of view in the View Configuration.
    m_colorSwapchainInfos.resize(m_viewConfigurationViews.size());
    m_depthSwapchainInfos.resize(m_viewConfigurationViews.size());

    // Per view, create a color and depth swapchain, and their associated image views.
    for (size_t i = 0; i < m_viewConfigurationViews.size(); i++) {
        SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];
        SwapchainInfo &depthSwapchainInfo = m_depthSwapchainInfos[i];

        // Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
        // Color.
        XrSwapchainCreateInfo swapchainCI{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.format = m_graphicsAPI->selectColorSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain);
        colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

        // Depth.
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        swapchainCI.format = m_graphicsAPI->selectDepthSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        xrCreateSwapchain(m_session, &swapchainCI, &depthSwapchainInfo.swapchain);
        depthSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

        // Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
        uint32_t colorSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr), "Failed to enumerate Color Swapchain Images.");
        XrSwapchainImageBaseHeader *colorSwapchainImages = m_graphicsAPI->allocateSwapchainImageData(colorSwapchainInfo.swapchain, GraphicsOpenGLES::SwapchainType::COLOR, colorSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages), "Failed to enumerate Color Swapchain Images.");

        uint32_t depthSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr), "Failed to enumerate Depth Swapchain Images.");
        XrSwapchainImageBaseHeader *depthSwapchainImages = m_graphicsAPI->allocateSwapchainImageData(depthSwapchainInfo.swapchain, GraphicsOpenGLES::SwapchainType::DEPTH, depthSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages), "Failed to enumerate Depth Swapchain Images.");

        // Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
        for (uint32_t j = 0; j < colorSwapchainImageCount; j++) {
            GraphicsOpenGLES::ImageViewCreateInfo imageViewCI{};
            imageViewCI.image = m_graphicsAPI->getSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCI.type = GraphicsOpenGLES::ImageViewCreateInfo::Type::RTV;
            imageViewCI.view = GraphicsOpenGLES::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCI.format = colorSwapchainInfo.swapchainFormat;
            imageViewCI.aspect = GraphicsOpenGLES::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(m_graphicsAPI->createImageView(imageViewCI));
        }
        for (uint32_t j = 0; j < depthSwapchainImageCount; j++) {
            GraphicsOpenGLES::ImageViewCreateInfo imageViewCI;
            imageViewCI.image = m_graphicsAPI->getSwapchainImage(depthSwapchainInfo.swapchain, j);
            imageViewCI.type = GraphicsOpenGLES::ImageViewCreateInfo::Type::DSV;
            imageViewCI.view = GraphicsOpenGLES::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCI.format = depthSwapchainInfo.swapchainFormat;
            imageViewCI.aspect = GraphicsOpenGLES::ImageViewCreateInfo::Aspect::DEPTH_BIT;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(m_graphicsAPI->createImageView(imageViewCI));
        }
    }
}

void VRApp::createResources() {
    // Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
    constexpr XrVector4f vertexPositions[] = {
            {+0.5f, +0.5f, +0.5f, 1.0f},
            {+0.5f, +0.5f, -0.5f, 1.0f},
            {+0.5f, -0.5f, +0.5f, 1.0f},
            {+0.5f, -0.5f, -0.5f, 1.0f},
            {-0.5f, +0.5f, +0.5f, 1.0f},
            {-0.5f, +0.5f, -0.5f, 1.0f},
            {-0.5f, -0.5f, +0.5f, 1.0f},
            {-0.5f, -0.5f, -0.5f, 1.0f}};

    XrVector4f cubeVertices[] = {
            CUBE_FACE(2, 1, 0, 2, 3, 1)  // -X
            CUBE_FACE(6, 4, 5, 6, 5, 7)  // +X
            CUBE_FACE(0, 1, 5, 0, 5, 4)  // -Y
            CUBE_FACE(2, 6, 7, 2, 7, 3)  // +Y
            CUBE_FACE(0, 4, 6, 0, 6, 2)  // -Z
            CUBE_FACE(1, 3, 7, 1, 7, 5)  // +Z
    };

    uint32_t cubeIndices[36] = {
            0, 1, 2, 3, 4, 5,        // -X
            6, 7, 8, 9, 10, 11,      // +X
            12, 13, 14, 15, 16, 17,  // -Y
            18, 19, 20, 21, 22, 23,  // +Y
            24, 25, 26, 27, 28, 29,  // -Z
            30, 31, 32, 33, 34, 35,  // +Z
    };

    m_vertexBuffer = m_graphicsAPI->createBuffer({GraphicsOpenGLES::BufferCreateInfo::Type::VERTEX, sizeof(float) * 4, sizeof(cubeVertices), &cubeVertices});

    m_indexBuffer = m_graphicsAPI->createBuffer({GraphicsOpenGLES::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(cubeIndices), &cubeIndices});

    size_t numberOfCuboids = 2;
    m_uniformBuffer_Camera = m_graphicsAPI->createBuffer({GraphicsOpenGLES::BufferCreateInfo::Type::UNIFORM, 0, sizeof(CameraConstants) * numberOfCuboids, nullptr});
    m_uniformBuffer_Normals = m_graphicsAPI->createBuffer({GraphicsOpenGLES::BufferCreateInfo::Type::UNIFORM, 0, sizeof(normals), &normals});

    std::string vertexSource = GraphicsOpenGLES::vertexShaderSource;
    m_vertexShader = m_graphicsAPI->createShader({GraphicsOpenGLES::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size()});
    std::string fragmentSource = GraphicsOpenGLES::fragmentShaderSource;
    m_fragmentShader = m_graphicsAPI->createShader({GraphicsOpenGLES::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size()});

    GraphicsOpenGLES::PipelineCreateInfo pipelineCI;
    pipelineCI.shaders = {m_vertexShader, m_fragmentShader};
    pipelineCI.vertexInputState.attributes = {{0, 0, GraphicsOpenGLES::VertexType::VEC4, 0, "TEXCOORD"}};
    pipelineCI.vertexInputState.bindings = {{0, 0, 4 * sizeof(float)}};
    pipelineCI.inputAssemblyState = {GraphicsOpenGLES::PrimitiveTopology::TRIANGLE_LIST, false};
    pipelineCI.rasterisationState = {false, false, GraphicsOpenGLES::PolygonMode::FILL, GraphicsOpenGLES::CullMode::BACK, GraphicsOpenGLES::FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f};
    pipelineCI.multisampleState = {1, false, 1.0f, 0xFFFFFFFF, false, false};
    pipelineCI.depthStencilState = {true, true, GraphicsOpenGLES::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f};
    pipelineCI.colorBlendState = {false, GraphicsOpenGLES::LogicOp::NO_OP, {{true, GraphicsOpenGLES::BlendFactor::SRC_ALPHA, GraphicsOpenGLES::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsOpenGLES::BlendOp::ADD, GraphicsOpenGLES::BlendFactor::ONE, GraphicsOpenGLES::BlendFactor::ZERO, GraphicsOpenGLES::BlendOp::ADD, (GraphicsOpenGLES::ColorComponentBit)15}}, {0.0f, 0.0f, 0.0f, 0.0f}};
    pipelineCI.colorFormats = {m_colorSwapchainInfos[0].swapchainFormat};
    pipelineCI.depthFormat = m_depthSwapchainInfos[0].swapchainFormat;
    pipelineCI.layout = {{0, nullptr, GraphicsOpenGLES::DescriptorInfo::Type::BUFFER, GraphicsOpenGLES::DescriptorInfo::Stage::VERTEX},
                         {1, nullptr, GraphicsOpenGLES::DescriptorInfo::Type::BUFFER, GraphicsOpenGLES::DescriptorInfo::Stage::VERTEX},
                         {2, nullptr, GraphicsOpenGLES::DescriptorInfo::Type::BUFFER, GraphicsOpenGLES::DescriptorInfo::Stage::FRAGMENT}};
    m_pipeline = m_graphicsAPI->createPipeline(pipelineCI);
}

void VRApp::pollSystemEvents() {
    // Checks whether Android has requested that application should by destroyed.
    if (androidApp->destroyRequested != 0) {
        m_applicationRunning = false;
        return;
    }
    while (true) {
        // Poll and process the Android OS system events.
        struct android_poll_source *source = nullptr;
        int events = 0;
        // The timeout depends on whether the application is active.
        const int timeoutMilliseconds = (!androidAppState.resumed && !m_sessionRunning && androidApp->destroyRequested == 0) ? -1 : 0;
        if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(androidApp, source);
            }
        } else {
            break;
        }
    }
}

void VRApp::pollEvents() {
    XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
    auto XrPollEvents = [&]() -> bool {
        eventData = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
    };

    while (XrPollEvents()) {
        switch (eventData.type) {
            // Log the number of lost events from the runtime.
            case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
                XrEventDataEventsLost *eventsLost = reinterpret_cast<XrEventDataEventsLost *>(&eventData);
                __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "OPENXR: Events Lost: ");
                break;
            }
                // Log that an instance loss is pending and shutdown the application.
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
                XrEventDataInstanceLossPending *instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending *>(&eventData);
                __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "OPENXR: Instance Loss Pending at: ");
                m_sessionRunning = false;
                m_applicationRunning = false;
                break;
            }
                // Log that the interaction profile has changed.
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
                XrEventDataInteractionProfileChanged *interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged *>(&eventData);
                __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",   "OPENXR: Interaction Profile changed for Session: ");
                if (interactionProfileChanged->session != m_session) {
                    __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "XrEventDataInteractionProfileChanged for unknown Session");
                    break;
                }
                break;
            }
                // Log that there's a reference space change pending.
            case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
                XrEventDataReferenceSpaceChangePending *referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending *>(&eventData);
                __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "OPENXR: Reference Space Change pending for Session: ");

                if (referenceSpaceChangePending->session != m_session) {
                    __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "XrEventDataReferenceSpaceChangePending for unknown Session");
                    break;
                }
                break;
            }
                // Session State changes:
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                XrEventDataSessionStateChanged *sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged *>(&eventData);
                if (sessionStateChanged->session != m_session) {
                    __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "XrEventDataSessionStateChanged for unknown Session");
                    break;
                }

                if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
                    // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
                    XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                    sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
                    OPENXR_CHECK(xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
                    m_sessionRunning = true;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
                    // SessionState is stopping. End the XrSession.
                    OPENXR_CHECK(xrEndSession(m_session), "Failed to end Session.");
                    m_sessionRunning = false;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
                    // SessionState is exiting. Exit the application.
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
                    // SessionState is loss pending. Exit the application.
                    // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }
                // Store state for reference across the application.
                m_sessionState = sessionStateChanged->state;
                break;
            }
            default: {
                break;
            }
        }
    }
}

void VRApp::renderFrame() {
    // Get the XrFrameState for timing and rendering info.
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");

    // Tell the OpenXR compositor that the application is beginning the frame.
    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

    // Variables for rendering and layer composition.
    bool rendered = false;
    RenderLayerInfo renderLayerInfo;
    renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

    // Check that the session is active and that we should render.
    bool sessionActive = (m_sessionState == XR_SESSION_STATE_SYNCHRONIZED || m_sessionState == XR_SESSION_STATE_VISIBLE || m_sessionState == XR_SESSION_STATE_FOCUSED);
    if (sessionActive && frameState.shouldRender) {
        // Render the stereo image and associate one of swapchain images with the XrCompositionLayerProjection structure.
        rendered = renderLayer(renderLayerInfo);
        if (rendered) {
            renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader *>(&renderLayerInfo.layerProjection));
        }
    }

    // Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = m_environmentBlendMode;
    frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
    frameEndInfo.layers = renderLayerInfo.layers.data();
    OPENXR_CHECK(xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");
}

void VRApp::destroySwapchains() {
    // Per view in the view configuration:
    for (size_t i = 0; i < m_viewConfigurationViews.size(); i++) {
        SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];
        SwapchainInfo &depthSwapchainInfo = m_depthSwapchainInfos[i];

        // Destroy the color and depth image views from GraphicsAPI.
        for (void *&imageView : colorSwapchainInfo.imageViews) {
            m_graphicsAPI->destroyImageView(imageView);
        }
        for (void *&imageView : depthSwapchainInfo.imageViews) {
            m_graphicsAPI->destroyImageView(imageView);
        }

        // Free the Swapchain Image Data.
        m_graphicsAPI->freeSwapchainImageData(colorSwapchainInfo.swapchain);
        m_graphicsAPI->freeSwapchainImageData(depthSwapchainInfo.swapchain);

        // Destroy the swapchains.
        OPENXR_CHECK(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain");
        OPENXR_CHECK(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy Depth Swapchain");
    }
}

void VRApp::destroyReferenceSpace(){
    OPENXR_CHECK(xrDestroySpace(m_localSpace), "Failed to destroy Space.")
}

void VRApp::destroyResources(){
    m_graphicsAPI->destroyPipeline(m_pipeline);
    m_graphicsAPI->destroyShader(m_fragmentShader);
    m_graphicsAPI->destroyShader(m_vertexShader);
    m_graphicsAPI->destroyBuffer(m_uniformBuffer_Camera);
    m_graphicsAPI->destroyBuffer(m_uniformBuffer_Normals);
    m_graphicsAPI->destroyBuffer(m_indexBuffer);
    m_graphicsAPI->destroyBuffer(m_vertexBuffer);
}

void VRApp::destroySession(){
    OPENXR_CHECK(xrDestroySession(m_session), "Failed to destroy Session.");
}

void VRApp::destroyInstance() {
    xrDestroyInstance(m_xrInstance);
}

bool VRApp::renderLayer(RenderLayerInfo &renderLayerInfo) {
    // Locate the views from the view configuration within the (reference) space at the display time.
    std::vector<XrView> views(m_viewConfigurationViews.size(), {XR_TYPE_VIEW});

    XrViewState viewState{XR_TYPE_VIEW_STATE};  // Will contain information on whether the position and/or orientation is valid and/or tracked.
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewLocateInfo.viewConfigurationType = m_viewConfiguration;
    viewLocateInfo.displayTime = renderLayerInfo.predictedDisplayTime;
    viewLocateInfo.space = m_localSpace;
    uint32_t viewCount = 0;
    XrResult locateViewsResult = xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
    if (locateViewsResult != XR_SUCCESS) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "Failed to locate Views.");
        return false;
    }
    // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
    renderLayerInfo.layerProjectionViews.resize(viewCount, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW});

    // Per view in the view configuration:
    for (uint32_t i = 0; i < viewCount; i++) {
        SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];
        SwapchainInfo &depthSwapchainInfo = m_depthSwapchainInfos[i];

        // Acquire and wait for an image from the swapchains.
        // Get the image index of an image in the swapchains.
        // The timeout is infinite.
        uint32_t colorImageIndex = 0;
        uint32_t depthImageIndex = 0;
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
        OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

        XrSwapchainImageWaitInfo waitInfo = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
        OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

        // Get the width and height and construct the viewport and scissors.
        const uint32_t &width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        const uint32_t &height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        GraphicsOpenGLES::Viewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
        GraphicsOpenGLES::Rect2D scissor = {{(int32_t)0, (int32_t)0}, {width, height}};
        float nearZ = 0.05f;
        float farZ = 100.0f;

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
        m_graphicsAPI->beginRendering();

        if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE) {
            // VR mode use a background color.
            m_graphicsAPI->clearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.0f, 0.0f, 0.0f, 1.00f);
        } else {
            // In AR mode make the background color black.
            m_graphicsAPI->clearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.00f, 0.00f, 0.00f, 1.00f);
        }
        m_graphicsAPI->clearDepth(depthSwapchainInfo.imageViews[depthImageIndex], 1.0f);

        m_graphicsAPI->setRenderAttachments(&colorSwapchainInfo.imageViews[colorImageIndex], 1, depthSwapchainInfo.imageViews[depthImageIndex], width, height, m_pipeline);
        m_graphicsAPI->setViewports(&viewport, 1);
        m_graphicsAPI->setScissors(&scissor, 1);

        // Compute the view-projection transform.
        // All matrices (including OpenXR's) are column-major, right-handed.
        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, views[i].fov, nearZ, farZ);
        XrMatrix4x4f toView;
        XrVector3f scale1m{1.0f, 1.0f, 1.0f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &views[i].pose.position, &views[i].pose.orientation, &scale1m);
        XrMatrix4x4f view;
        XrMatrix4x4f_InvertRigidBody(&view, &toView);
        XrMatrix4x4f_Multiply(&cameraConstants.viewProj, &proj, &view);

        renderCuboidIndex = 0;
        // Draw a floor. Scale it by 2 in the X and Z, and 0.1 in the Y,
        renderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f}}, {2.0f, 0.1f, 2.0f}, {0.4f, 0.5f, 0.5f});
        // Draw a "table".
        renderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f}}, {1.0f, 0.2f, 1.0f}, {0.6f, 0.6f, 0.4f});

        testGUI.render();

        m_graphicsAPI->endRendering();

        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
        OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");
    }

    // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
    renderLayerInfo.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.layerProjection.space = m_localSpace;
    renderLayerInfo.layerProjection.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.layerProjection.views = renderLayerInfo.layerProjectionViews.data();

    return true;
}

void VRApp::renderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color) {
    XrMatrix4x4f_CreateTranslationRotationScale(&cameraConstants.model, &pose.position, &pose.orientation, &scale);

    XrMatrix4x4f_Multiply(&cameraConstants.modelViewProj, &cameraConstants.viewProj, &cameraConstants.model);
    cameraConstants.color = {color.x, color.y, color.z, 1.0};
    size_t offsetCameraUB = sizeof(CameraConstants) * renderCuboidIndex;

    m_graphicsAPI->setPipeline(m_pipeline);

    m_graphicsAPI->setBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CameraConstants), &cameraConstants);
    m_graphicsAPI->setDescriptor({0, m_uniformBuffer_Camera, GraphicsOpenGLES::DescriptorInfo::Type::BUFFER, GraphicsOpenGLES::DescriptorInfo::Stage::VERTEX, false, offsetCameraUB, sizeof(CameraConstants)});
    m_graphicsAPI->setDescriptor({1, m_uniformBuffer_Normals, GraphicsOpenGLES::DescriptorInfo::Type::BUFFER, GraphicsOpenGLES::DescriptorInfo::Stage::VERTEX, false, 0, sizeof(normals)});

    m_graphicsAPI->setVertexBuffers(&m_vertexBuffer, 1);
    m_graphicsAPI->setIndexBuffer(m_indexBuffer);
    m_graphicsAPI->drawIndexed(36);

    renderCuboidIndex++;
}