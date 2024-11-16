#include "GraphicsOpenGLES.h"

const char* GraphicsOpenGLES::vertexShaderSource = R"(
#version 310 es

layout(std140, binding = 0) uniform CameraConstants {
    mat4 viewProj;
    mat4 modelViewProj;
    mat4 model;
    vec4 colour;
    vec4 pad1;
    vec4 pad2;
    vec4 pad3;
};
layout(std140, binding = 1) uniform Normals {
    vec4 normals[6];
};
layout(location = 0) in highp vec4 a_Positions;
layout(location = 0) out flat uvec2 o_TexCoord;
layout(location = 1) out highp vec3 o_Normal;
layout(location = 2) out flat vec3 o_Colour;
void main() {
    gl_Position = modelViewProj * a_Positions;
    int face = gl_VertexID / 6;
    o_TexCoord = uvec2(face, 0);
    o_Normal = (model * normals[face]).xyz;
    o_Colour = vec3(1,0,0);//colour.rgb;
}
)";

const char* GraphicsOpenGLES::fragmentShaderSource = R"(
#version 310 es

layout(location = 0) in flat uvec2 i_TexCoord;
layout(location = 1) in highp vec3 i_Normal;
layout(location = 2) in flat highp vec3 i_Color;
layout(location = 0) out highp vec4 o_Color;
layout(std140, binding = 2) uniform Data {
    highp vec4 colors[6];
} d_Data;

void main() {
    uint i = i_TexCoord.x;
    highp float light = 0.1 + 0.9 * clamp(i_Normal.g, 0.0, 1.0);
    o_Color = highp vec4(light * i_Color.rgb, 1.0);
}
)";

GraphicsOpenGLES::GraphicsOpenGLES(XrInstance m_xrInstance, XrSystemId systemId) {
    OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetOpenGLESGraphicsRequirementsKHR", (PFN_xrVoidFunction *)&xrGetOpenGLESGraphicsRequirementsKHR), "Failed to get InstanceProcAddr for xrGetOpenGLESGraphicsRequirementsKHR.");
    XrGraphicsRequirementsOpenGLESKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR};
    // CRASH HERE
    OPENXR_CHECK(xrGetOpenGLESGraphicsRequirementsKHR(m_xrInstance, systemId, &graphicsRequirements), "Failed to get Graphics Requirements for OpenGLES.");

    // Create an EGL display
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "ERROR: Failed to get default EGL display." << std::endl;
        return;
    }

    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        std::cerr << "ERROR: Failed to initialize EGL." << std::endl;
        return;
    }

    // Choose a suitable EGL configuration
    const EGLint configAttributes[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
    };
    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfigs) || numConfigs == 0) {
        std::cerr << "ERROR: Failed to choose a valid EGLConfig." << std::endl;
        return;
    }

    // Create a Pbuffer surface for offscreen rendering
    const EGLint pbufferAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbufferAttributes);
    if (eglSurface == EGL_NO_SURFACE) {
        std::cerr << "ERROR: Failed to create EGL Pbuffer surface." << std::endl;
        return;
    }

    // Create an EGL context
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT,
                                  (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE});
    if (eglContext == EGL_NO_CONTEXT) {
        std::cerr << "ERROR: Failed to create EGL context." << std::endl;
        return;
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        std::cerr << "ERROR: Failed to make EGL context current." << std::endl;
        return;
    }
}


const std::vector<int64_t> GraphicsOpenGLES::getSupportedDepthSwapchainFormats() {
    return {
            GL_DEPTH_COMPONENT32F,
            GL_DEPTH_COMPONENT24,
            GL_DEPTH_COMPONENT16
    };
}

void* GraphicsOpenGLES::getSwapchainImage(XrSwapchain swapchain, uint32_t index) {
    return (void*)(uint64_t)swapchainImagesMap[swapchain].second[index].image;
}

int64_t GraphicsOpenGLES::selectColorSwapchainFormat(const std::vector<int64_t> &formats) {
    const std::vector<int64_t> &supportSwapchainFormats = getSupportedColorSwapchainFormats();

    const std::vector<int64_t>::const_iterator &swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
                                                                                       std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
    if (swapchainFormatIt == formats.end()) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s","ERROR: Unable to find supported Color Swapchain Format");
        return 0;
    }

    return *swapchainFormatIt;
}

void GraphicsOpenGLES::setPipeline(void *pipeline) {
    GLuint program = (GLuint)(uint64_t)pipeline;
    glUseProgram(program);
    m_setPipeline = program;

    const PipelineCreateInfo &pipelineCI = pipelines[program];

    // InputAssemblyState
    const InputAssemblyState &IAS = pipelineCI.inputAssemblyState;
    if (IAS.primitiveRestartEnable) {
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    } else {
        glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    }

    // RasterisationState
    const RasterisationState &RS = pipelineCI.rasterisationState;

    if (RS.rasteriserDiscardEnable) {
        glEnable(GL_RASTERIZER_DISCARD);
    } else {
        glDisable(GL_RASTERIZER_DISCARD);
    }

    if (RS.cullMode > CullMode::NONE) {
        glEnable(GL_CULL_FACE);
        glCullFace(ToGLCullMode(RS.cullMode));
    } else {
        glDisable(GL_CULL_FACE);
    }

    glFrontFace(RS.frontFace == FrontFace::COUNTER_CLOCKWISE ? GL_CCW : GL_CW);

    GLenum polygonOffsetMode = 0;
    switch (RS.polygonMode) {
        default:
        case PolygonMode::FILL: {
            polygonOffsetMode = GL_POLYGON_OFFSET_FILL;
            break;
        }
        case PolygonMode::LINE: {
            polygonOffsetMode = 0; //GL_POLYGON_OFFSET_LINE; //None for ES
            break;
        }
        case PolygonMode::POINT: {
            polygonOffsetMode = 0; //GL_POLYGON_OFFSET_POINT; //None for ES
            break;
        }
    }
    if (RS.depthBiasEnable) {
        glEnable(polygonOffsetMode);
        // glPolygonOffsetClamp
        glPolygonOffset(RS.depthBiasSlopeFactor, RS.depthBiasConstantFactor);
    } else {
        glDisable(polygonOffsetMode);
    }

    glLineWidth(RS.lineWidth);

    // MultisampleState
    const MultisampleState &MS = pipelineCI.multisampleState;

    if (MS.sampleShadingEnable) {
        glEnable(GL_SAMPLE_SHADING);
        glMinSampleShading(MS.minSampleShading);
    } else {
        glDisable(GL_SAMPLE_SHADING);
    }

    if (MS.sampleMask > 0) {
        glEnable(GL_SAMPLE_MASK);
        glSampleMaski(0, MS.sampleMask);
    } else {
        glDisable(GL_SAMPLE_MASK);
    }

    if (MS.alphaToCoverageEnable) {
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    } else {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }

    // DepthStencilState
    const DepthStencilState &DSS = pipelineCI.depthStencilState;

    if (DSS.depthTestEnable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(DSS.depthWriteEnable ? GL_TRUE : GL_FALSE);

    glDepthFunc(ToGLCompareOp(DSS.depthCompareOp));

    if (DSS.stencilTestEnable) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }

    glStencilOpSeparate(GL_FRONT,
                        ToGLStencilCompareOp(DSS.front.failOp),
                        ToGLStencilCompareOp(DSS.front.depthFailOp),
                        ToGLStencilCompareOp(DSS.front.passOp));
    glStencilFuncSeparate(GL_FRONT,
                          ToGLCompareOp(DSS.front.compareOp),
                          DSS.front.reference,
                          DSS.front.compareMask);
    glStencilMaskSeparate(GL_FRONT, DSS.front.writeMask);

    glStencilOpSeparate(GL_BACK,
                        ToGLStencilCompareOp(DSS.back.failOp),
                        ToGLStencilCompareOp(DSS.back.depthFailOp),
                        ToGLStencilCompareOp(DSS.back.passOp));
    glStencilFuncSeparate(GL_BACK,
                          ToGLCompareOp(DSS.back.compareOp),
                          DSS.back.reference,
                          DSS.back.compareMask);
    glStencilMaskSeparate(GL_BACK, DSS.back.writeMask);

    // ColorBlendState
    const ColorBlendState &CBS = pipelineCI.colorBlendState;

    for (int i = 0; i < (int)CBS.attachments.size(); i++) {
        const ColorBlendAttachmentState &CBA = CBS.attachments[i];

        if (CBA.blendEnable) {
            glEnablei(GL_BLEND, i);
        } else {
            glDisablei(GL_BLEND, i);
        }

        glBlendEquationSeparatei(i, ToGLBlendOp(CBA.colorBlendOp), ToGLBlendOp(CBA.alphaBlendOp));

        glBlendFuncSeparatei(i,
                             ToGLBlendFactor(CBA.srcColorBlendFactor),
                             ToGLBlendFactor(CBA.dstColorBlendFactor),
                             ToGLBlendFactor(CBA.srcAlphaBlendFactor),
                             ToGLBlendFactor(CBA.dstAlphaBlendFactor));

        glColorMaski(i,
                     (((uint32_t)CBA.colorWriteMask & (uint32_t)ColorComponentBit::R_BIT) == (uint32_t)ColorComponentBit::R_BIT),
                     (((uint32_t)CBA.colorWriteMask & (uint32_t)ColorComponentBit::G_BIT) == (uint32_t)ColorComponentBit::G_BIT),
                     (((uint32_t)CBA.colorWriteMask & (uint32_t)ColorComponentBit::B_BIT) == (uint32_t)ColorComponentBit::B_BIT),
                     (((uint32_t)CBA.colorWriteMask & (uint32_t)ColorComponentBit::A_BIT) == (uint32_t)ColorComponentBit::A_BIT));
    }
    glBlendColor(CBS.blendConstants[0], CBS.blendConstants[1], CBS.blendConstants[2], CBS.blendConstants[3]);
}


const std::vector<int64_t> GraphicsOpenGLES::getSupportedColorSwapchainFormats() {
    GLint glMajorVersion = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    if (glMajorVersion >= 3) {
        return {GL_RGBA8, GL_RGBA8_SNORM, GL_SRGB8_ALPHA8};
    } else {
        return {GL_RGBA8, GL_RGBA8_SNORM};
    }
}

void* GraphicsOpenGLES::createImageView(const ImageViewCreateInfo &imageViewCI) {
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);

    GLenum attachment = imageViewCI.aspect == ImageViewCreateInfo::Aspect::COLOR_BIT ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D_ARRAY) {
        //glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, attachment, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel, imageViewCI.baseArrayLayer, imageViewCI.layerCount);
    } else if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D) {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s","ERROR: OPENGL: Unknown ImageView View type.");
    }

    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        //DEBUG_BREAK; // running into GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT  error here
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Framebuffer is not complete" );
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    imageViews[framebuffer] = imageViewCI;
    return (void *)(uint64_t)framebuffer;
}

int64_t GraphicsOpenGLES::selectDepthSwapchainFormat(const std::vector<int64_t> &formats) {
    const std::vector<int64_t> &supportSwapchainFormats = getSupportedDepthSwapchainFormats();

    const std::vector<int64_t>::const_iterator &swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
                                                                                       std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
    if (swapchainFormatIt == formats.end()) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "ERROR: Unable to find supported Depth Swapchain Format" );
        return 0;
    }

    return *swapchainFormatIt;
}

GLenum GraphicsOpenGLES::getGLTextureTarget(const ImageCreateInfo &imageCI) {
    GLenum target = 0;
    if (imageCI.dimension == 1) {
        if (imageCI.arrayLayers > 1) {
            target = GL_TEXTURE_1D_ARRAY;
        } else {
            target = GL_TEXTURE_1D;
        }
    } else if (imageCI.dimension == 2) {
        if (imageCI.cubemap) {
            if (imageCI.arrayLayers > 6) {
                target = GL_TEXTURE_CUBE_MAP_ARRAY;
            } else {
                target = GL_TEXTURE_CUBE_MAP;
            }
        } else {
            if (imageCI.sampleCount > 1) {
                if (imageCI.arrayLayers > 1) {
                    target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
                } else {
                    target = GL_TEXTURE_2D_MULTISAMPLE;
                }
            } else {
                if (imageCI.arrayLayers > 1) {
                    target = GL_TEXTURE_2D_ARRAY;
                } else {
                    target = GL_TEXTURE_2D;
                }
            }
        }
    } else if (imageCI.dimension == 3) {
        target = GL_TEXTURE_3D;
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "ERROR: OPENGL: Unknown Dimension for getGLTextureTarget(): " );
    }
    return target;
}

void GraphicsOpenGLES::setVertexBuffers(void **vertexBuffers, size_t count) {
    const VertexInputState &vertexInputState = pipelines[m_setPipeline].vertexInputState;
    for (size_t i = 0; i < count; i++) {
        GLuint glVertexBufferID = (GLuint)(uint64_t)vertexBuffers[i];
        if (buffers[glVertexBufferID].type != BufferCreateInfo::Type::VERTEX) {
            __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s", "ERROR: OpenGL: Provided buffer is not type: VERTEX." );
        }

        glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uint64_t)vertexBuffers[i]);

        for (const VertexInputBinding &vertexBinding : vertexInputState.bindings) {
            if (vertexBinding.bindingIndex == (uint32_t)i) {
                for (const VertexInputAttribute &vertexAttribute : vertexInputState.attributes) {
                    if (vertexAttribute.bindingIndex == (uint32_t)i) {
                        GLuint attribIndex = vertexAttribute.attribIndex;
                        GLint size = ((GLint)vertexAttribute.vertexType % 4) + 1;
                        GLenum type = (GLenum)vertexAttribute.vertexType >= (GLenum)VertexType::UINT ? GL_UNSIGNED_INT : (GLenum)vertexAttribute.vertexType >= (GLenum)VertexType::INT ? GL_INT
                                                                                                                                                                                       : GL_FLOAT;
                        GLsizei stride = vertexBinding.stride;
                        const void *offset = (const void *)vertexAttribute.offset;
                        glEnableVertexAttribArray(attribIndex);
                        glVertexAttribPointer(attribIndex, size, type, false, stride, offset);
                    }
                }
            }
        }
    }
}

void GraphicsOpenGLES::setIndexBuffer(void *indexBuffer) {
    GLuint glIndexBufferID = (GLuint)(uint64_t)indexBuffer;
    if (buffers[glIndexBufferID].type != BufferCreateInfo::Type::INDEX) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",   "ERROR: OpenGL: Provided buffer is not type: INDEX." );
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBufferID);
    m_setIndexBuffer = glIndexBufferID;
}

void GraphicsOpenGLES::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    GLenum indexType = buffers[m_setIndexBuffer].stride == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    glDrawElementsInstanced(ToGLTopology(pipelines[m_setPipeline].inputAssemblyState.topology),indexCount, indexType, nullptr,instanceCount);
}

void GraphicsOpenGLES::setDescriptor(const DescriptorInfo &descriptorInfo) {
    GLuint glResource = (GLuint)(uint64_t)descriptorInfo.resource;
    const GLuint &bindingIndex = descriptorInfo.bindingIndex;
    if (descriptorInfo.type == DescriptorInfo::Type::BUFFER) {
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, glResource, (GLintptr)descriptorInfo.bufferOffset, (GLsizeiptr)descriptorInfo.bufferSize);
    } else if (descriptorInfo.type == DescriptorInfo::Type::IMAGE) {
        glActiveTexture(GL_TEXTURE0 + bindingIndex);
        glBindTexture(getGLTextureTarget(images[glResource]), glResource);
    } else if (descriptorInfo.type == DescriptorInfo::Type::SAMPLER) {
        glBindSampler(bindingIndex, glResource);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",   "ERROR: OPENGL: Unknown Descriptor Type." );
    }
}

void GraphicsOpenGLES::setBufferData(void *buffer, size_t offset, size_t size, void *data) {
    GLuint glBuffer = (GLuint)(uint64_t)buffer;
    const BufferCreateInfo &bufferCI = buffers[glBuffer];

    GLenum target = 0;
    if (bufferCI.type == BufferCreateInfo::Type::VERTEX) {
        target = GL_ARRAY_BUFFER;
    } else if (bufferCI.type == BufferCreateInfo::Type::INDEX) {
        target = GL_ELEMENT_ARRAY_BUFFER;
    } else if (bufferCI.type == BufferCreateInfo::Type::UNIFORM) {
        target = GL_UNIFORM_BUFFER;
    } else {
        //EBUG_BREAK;
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Unknown Buffer Type." );
    }

    if (data) {
        glBindBuffer(target, glBuffer);
        glBufferSubData(target, (GLintptr)offset, (GLsizeiptr)size, data);
        glBindBuffer(target, 0);
    }
}

void GraphicsOpenGLES::setRenderAttachments(void **colorViews, size_t colorViewCount, void *depthStencilView, uint32_t width, uint32_t height, void *pipeline) {
    // Reset Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &setFramebuffer);
    setFramebuffer = 0;

    glGenFramebuffers(1, &setFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, setFramebuffer);

    // Color
    for (size_t i = 0; i < colorViewCount; i++) {
        GLenum attachment = GL_COLOR_ATTACHMENT0;

        GLuint glColorView = (GLuint)(uint64_t)colorViews[i];
        const ImageViewCreateInfo &imageViewCI = imageViews[glColorView];

        if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D_ARRAY) {
            //glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel, imageViewCI.baseArrayLayer, imageViewCI.layerCount);
        } else if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel);
        } else {
            //DEBUG_BREAK;
            __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Unknown ImageView View type." );
        }
    }
    // DepthStencil
    if (depthStencilView) {
        GLuint glDepthView = (GLuint)(uint64_t)depthStencilView;
        const ImageViewCreateInfo &imageViewCI = imageViews[glDepthView];

        if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D_ARRAY) {
            //glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel, imageViewCI.baseArrayLayer, imageViewCI.layerCount);
        } else if (imageViewCI.view == ImageViewCreateInfo::View::TYPE_2D) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel);
        } else {
            //DEBUG_BREAK;
            __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Unknown ImageView View type." );
        }
    }

    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Framebuffer is not complete." );
    }
}

void* GraphicsOpenGLES::getGraphicsBinding() {
    static XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR};
    graphicsBinding.display = this->eglDisplay;
    graphicsBinding.config = this->eglConfig;
    graphicsBinding.context = this->eglContext;
    return &graphicsBinding;
}

void GraphicsOpenGLES::setViewports(Viewport *viewports, size_t count) {
    Viewport viewport = viewports[0];
    glViewport((GLint)viewport.x, (GLint)viewport.y, (GLsizei)viewport.width, (GLsizei)viewport.height);
    glDepthRangef(viewport.minDepth, viewport.maxDepth);
}

void GraphicsOpenGLES::setScissors(Rect2D *scissors, size_t count) {
    Rect2D scissor = scissors[0];
    glScissor((GLint)scissor.offset.x, (GLint)scissor.offset.y, (GLsizei)scissor.extent.width, (GLsizei)scissor.extent.height);
}

void GraphicsOpenGLES::beginRendering() {
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glGenFramebuffers(1, &setFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, setFramebuffer);
}

void GraphicsOpenGLES::endRendering() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &setFramebuffer);
    setFramebuffer = 0;

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vertexArray);
    vertexArray = 0;
}

void* GraphicsOpenGLES::createPipeline(const PipelineCreateInfo &pipelineCI) {
    GLuint program = glCreateProgram();

    for (const void *const &shader : pipelineCI.shaders)
        glAttachShader(program, (GLuint)(uint64_t)shader);

    glLinkProgram(program);
    glValidateProgram(program);

    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        glDeleteProgram(program);
    }

    for (const void *const &shader : pipelineCI.shaders)
        glDetachShader(program, (GLuint)(uint64_t)shader);

    pipelines[program] = pipelineCI;

    return (void *)(uint64_t)program;
}

void GraphicsOpenGLES::clearColor(void *imageView, float r, float g, float b, float a) {
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)(uint64_t)imageView);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsOpenGLES::clearDepth(void *imageView, float d) {
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)(uint64_t)imageView);
    glClearDepthf(d);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void* GraphicsOpenGLES::createBuffer(const BufferCreateInfo &bufferCI) {
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);

    GLenum target = 0;
    if (bufferCI.type == BufferCreateInfo::Type::VERTEX) {
        target = GL_ARRAY_BUFFER;
    } else if (bufferCI.type == BufferCreateInfo::Type::INDEX) {
        target = GL_ELEMENT_ARRAY_BUFFER;
    } else if (bufferCI.type == BufferCreateInfo::Type::UNIFORM) {
        target = GL_UNIFORM_BUFFER;
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Unknown Buffer Type." );
    }

    glBindBuffer(target, buffer);
    glBufferData(target, (GLsizeiptr)bufferCI.size, bufferCI.data, GL_STATIC_DRAW);
    glBindBuffer(target, 0);

    buffers[buffer] = bufferCI;
    return (void *)(uint64_t)buffer;
}

void* GraphicsOpenGLES::createShader(const ShaderCreateInfo &shaderCI) {
    GLenum type = 0;
    switch (shaderCI.type) {
        case ShaderCreateInfo::Type::VERTEX: {
            type = GL_VERTEX_SHADER;
            break;
        }
        case ShaderCreateInfo::Type::TESSELLATION_CONTROL: {
            type = GL_TESS_CONTROL_SHADER;
            break;
        }
        case ShaderCreateInfo::Type::TESSELLATION_EVALUATION: {
            type = GL_TESS_EVALUATION_SHADER;
            break;
        }
        case ShaderCreateInfo::Type::GEOMETRY: {
            type = GL_GEOMETRY_SHADER;
            break;
        }
        case ShaderCreateInfo::Type::FRAGMENT: {
            type = GL_FRAGMENT_SHADER;
            break;
        }
        case ShaderCreateInfo::Type::COMPUTE: {
            type = GL_COMPUTE_SHADER;
            break;
        }
        default:
            __android_log_print(ANDROID_LOG_ERROR, "VRAPP", "%s",  "ERROR: OPENGL: Unknown Shader Type." );
    }
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &shaderCI.sourceData, nullptr);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        //std::cout << infoLog.data() << std::endl;

        glDeleteShader(shader);
        shader = 0;
    }
    return (void *)(uint64_t)shader;
}

XrSwapchainImageBaseHeader* GraphicsOpenGLES::allocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) {
    swapchainImagesMap[swapchain].first = type;
    swapchainImagesMap[swapchain].second.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR});
    return reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImagesMap[swapchain].second.data());
}

void GraphicsOpenGLES::destroyImageView(void *&imageView) {
    GLuint framebuffer = (GLuint)(uint64_t)imageView;
    imageViews.erase(framebuffer);
    glDeleteFramebuffers(1, &framebuffer);
    imageView = nullptr;
}

void GraphicsOpenGLES::freeSwapchainImageData(XrSwapchain swapchain) {
    swapchainImagesMap[swapchain].second.clear();
    swapchainImagesMap.erase(swapchain);
}

void GraphicsOpenGLES::destroyPipeline(void *&pipeline) {
    GLint program = (GLuint)(uint64_t)pipeline;
    pipelines.erase(program);
    glDeleteProgram(program);
    pipeline = nullptr;
}

void GraphicsOpenGLES::destroyShader(void *&shader) {
    GLuint glShader = (GLuint)(uint64_t)shader;
    glDeleteShader(glShader);
    shader = nullptr;
}

void GraphicsOpenGLES::destroyBuffer(void *&buffer) {
    GLuint glBuffer = (GLuint)(uint64_t)buffer;
    buffers.erase(glBuffer);
    glDeleteBuffers(1, &glBuffer);
    buffer = nullptr;
}