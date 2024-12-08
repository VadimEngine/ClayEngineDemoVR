#pragma once

#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES
#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

// standard lib
#include <string>
#include <vector>
#include <unordered_map>
// third party
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "XRUtils.h"

// No 1D textures in OpenGL ES.
#if !defined(GL_TEXTURE_1D)
#define GL_TEXTURE_1D 0x0DE0
#endif

// No 1D texture arrays in OpenGL ES.
#if !defined(GL_TEXTURE_1D_ARRAY)
#define GL_TEXTURE_1D_ARRAY 0x8C18
#endif

typedef struct XrMatrix4x4f {
    float m[16];
} XrMatrix4x4f;

// Creates a scale matrix.
inline static void XrMatrix4x4f_CreateScale(XrMatrix4x4f* result, const float x, const float y, const float z) {
    result->m[0] = x;
    result->m[1] = 0.0f;
    result->m[2] = 0.0f;
    result->m[3] = 0.0f;
    result->m[4] = 0.0f;
    result->m[5] = y;
    result->m[6] = 0.0f;
    result->m[7] = 0.0f;
    result->m[8] = 0.0f;
    result->m[9] = 0.0f;
    result->m[10] = z;
    result->m[11] = 0.0f;
    result->m[12] = 0.0f;
    result->m[13] = 0.0f;
    result->m[14] = 0.0f;
    result->m[15] = 1.0f;
}

inline static void XrMatrix4x4f_CreateFromQuaternion(XrMatrix4x4f* result, const XrQuaternionf* quat) {
    const float x2 = quat->x + quat->x;
    const float y2 = quat->y + quat->y;
    const float z2 = quat->z + quat->z;

    const float xx2 = quat->x * x2;
    const float yy2 = quat->y * y2;
    const float zz2 = quat->z * z2;

    const float yz2 = quat->y * z2;
    const float wx2 = quat->w * x2;
    const float xy2 = quat->x * y2;
    const float wz2 = quat->w * z2;
    const float xz2 = quat->x * z2;
    const float wy2 = quat->w * y2;

    result->m[0] = 1.0f - yy2 - zz2;
    result->m[1] = xy2 + wz2;
    result->m[2] = xz2 - wy2;
    result->m[3] = 0.0f;

    result->m[4] = xy2 - wz2;
    result->m[5] = 1.0f - xx2 - zz2;
    result->m[6] = yz2 + wx2;
    result->m[7] = 0.0f;

    result->m[8] = xz2 + wy2;
    result->m[9] = yz2 - wx2;
    result->m[10] = 1.0f - xx2 - yy2;
    result->m[11] = 0.0f;

    result->m[12] = 0.0f;
    result->m[13] = 0.0f;
    result->m[14] = 0.0f;
    result->m[15] = 1.0f;
}

inline static void XrMatrix4x4f_CreateTranslation(XrMatrix4x4f* result, const float x, const float y, const float z) {
    result->m[0] = 1.0f;
    result->m[1] = 0.0f;
    result->m[2] = 0.0f;
    result->m[3] = 0.0f;
    result->m[4] = 0.0f;
    result->m[5] = 1.0f;
    result->m[6] = 0.0f;
    result->m[7] = 0.0f;
    result->m[8] = 0.0f;
    result->m[9] = 0.0f;
    result->m[10] = 1.0f;
    result->m[11] = 0.0f;
    result->m[12] = x;
    result->m[13] = y;
    result->m[14] = z;
    result->m[15] = 1.0f;
}

// Calculates the inverse of a rigid body transform.
inline static void XrMatrix4x4f_InvertRigidBody(XrMatrix4x4f* result, const XrMatrix4x4f* src) {
    result->m[0] = src->m[0];
    result->m[1] = src->m[4];
    result->m[2] = src->m[8];
    result->m[3] = 0.0f;
    result->m[4] = src->m[1];
    result->m[5] = src->m[5];
    result->m[6] = src->m[9];
    result->m[7] = 0.0f;
    result->m[8] = src->m[2];
    result->m[9] = src->m[6];
    result->m[10] = src->m[10];
    result->m[11] = 0.0f;
    result->m[12] = -(src->m[0] * src->m[12] + src->m[1] * src->m[13] + src->m[2] * src->m[14]);
    result->m[13] = -(src->m[4] * src->m[12] + src->m[5] * src->m[13] + src->m[6] * src->m[14]);
    result->m[14] = -(src->m[8] * src->m[12] + src->m[9] * src->m[13] + src->m[10] * src->m[14]);
    result->m[15] = 1.0f;
}

// Use left-multiplication to accumulate transformations.
inline static void XrMatrix4x4f_Multiply(XrMatrix4x4f* result, const XrMatrix4x4f* a, const XrMatrix4x4f* b) {
    result->m[0] = a->m[0] * b->m[0] + a->m[4] * b->m[1] + a->m[8] * b->m[2] + a->m[12] * b->m[3];
    result->m[1] = a->m[1] * b->m[0] + a->m[5] * b->m[1] + a->m[9] * b->m[2] + a->m[13] * b->m[3];
    result->m[2] = a->m[2] * b->m[0] + a->m[6] * b->m[1] + a->m[10] * b->m[2] + a->m[14] * b->m[3];
    result->m[3] = a->m[3] * b->m[0] + a->m[7] * b->m[1] + a->m[11] * b->m[2] + a->m[15] * b->m[3];

    result->m[4] = a->m[0] * b->m[4] + a->m[4] * b->m[5] + a->m[8] * b->m[6] + a->m[12] * b->m[7];
    result->m[5] = a->m[1] * b->m[4] + a->m[5] * b->m[5] + a->m[9] * b->m[6] + a->m[13] * b->m[7];
    result->m[6] = a->m[2] * b->m[4] + a->m[6] * b->m[5] + a->m[10] * b->m[6] + a->m[14] * b->m[7];
    result->m[7] = a->m[3] * b->m[4] + a->m[7] * b->m[5] + a->m[11] * b->m[6] + a->m[15] * b->m[7];

    result->m[8] = a->m[0] * b->m[8] + a->m[4] * b->m[9] + a->m[8] * b->m[10] + a->m[12] * b->m[11];
    result->m[9] = a->m[1] * b->m[8] + a->m[5] * b->m[9] + a->m[9] * b->m[10] + a->m[13] * b->m[11];
    result->m[10] = a->m[2] * b->m[8] + a->m[6] * b->m[9] + a->m[10] * b->m[10] + a->m[14] * b->m[11];
    result->m[11] = a->m[3] * b->m[8] + a->m[7] * b->m[9] + a->m[11] * b->m[10] + a->m[15] * b->m[11];

    result->m[12] = a->m[0] * b->m[12] + a->m[4] * b->m[13] + a->m[8] * b->m[14] + a->m[12] * b->m[15];
    result->m[13] = a->m[1] * b->m[12] + a->m[5] * b->m[13] + a->m[9] * b->m[14] + a->m[13] * b->m[15];
    result->m[14] = a->m[2] * b->m[12] + a->m[6] * b->m[13] + a->m[10] * b->m[14] + a->m[14] * b->m[15];
    result->m[15] = a->m[3] * b->m[12] + a->m[7] * b->m[13] + a->m[11] * b->m[14] + a->m[15] * b->m[15];
}

inline static void XrMatrix4x4f_CreateTranslationRotationScale(XrMatrix4x4f* result, const XrVector3f* translation,
                                                               const XrQuaternionf* rotation, const XrVector3f* scale) {
    XrMatrix4x4f scaleMatrix;
    XrMatrix4x4f_CreateScale(&scaleMatrix, scale->x, scale->y, scale->z);

    XrMatrix4x4f rotationMatrix;
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, rotation);

    XrMatrix4x4f translationMatrix;
    XrMatrix4x4f_CreateTranslation(&translationMatrix, translation->x, translation->y, translation->z);

    XrMatrix4x4f combinedMatrix;
    XrMatrix4x4f_Multiply(&combinedMatrix, &rotationMatrix, &scaleMatrix);
    XrMatrix4x4f_Multiply(result, &translationMatrix, &combinedMatrix);
}

inline static void XrMatrix4x4f_CreateProjection(XrMatrix4x4f* result, const float tanAngleLeft,
                                                 const float tanAngleRight, const float tanAngleUp, float const tanAngleDown,
                                                 const float nearZ, const float farZ) {
    const float tanAngleWidth = tanAngleRight - tanAngleLeft;

    // Set to tanAngleDown - tanAngleUp for a clip space with positive Y down (Vulkan).
    // Set to tanAngleUp - tanAngleDown for a clip space with positive Y up (OpenGL / D3D / Metal).
    const float tanAngleHeight = (tanAngleUp - tanAngleDown);

    // Set to nearZ for a [-1,1] Z clip space (OpenGL / OpenGL ES).
    // Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
    const float offsetZ = nearZ;

    if (farZ <= nearZ) {
        // place the far plane at infinity
        result->m[0] = 2.0f / tanAngleWidth;
        result->m[4] = 0.0f;
        result->m[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result->m[12] = 0.0f;

        result->m[1] = 0.0f;
        result->m[5] = 2.0f / tanAngleHeight;
        result->m[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result->m[13] = 0.0f;

        result->m[2] = 0.0f;
        result->m[6] = 0.0f;
        result->m[10] = -1.0f;
        result->m[14] = -(nearZ + offsetZ);

        result->m[3] = 0.0f;
        result->m[7] = 0.0f;
        result->m[11] = -1.0f;
        result->m[15] = 0.0f;
    } else {
        // normal projection
        result->m[0] = 2.0f / tanAngleWidth;
        result->m[4] = 0.0f;
        result->m[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result->m[12] = 0.0f;

        result->m[1] = 0.0f;
        result->m[5] = 2.0f / tanAngleHeight;
        result->m[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result->m[13] = 0.0f;

        result->m[2] = 0.0f;
        result->m[6] = 0.0f;
        result->m[10] = -(farZ + offsetZ) / (farZ - nearZ);
        result->m[14] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

        result->m[3] = 0.0f;
        result->m[7] = 0.0f;
        result->m[11] = -1.0f;
        result->m[15] = 0.0f;
    }
}

inline static void XrMatrix4x4f_CreateProjectionFov(XrMatrix4x4f* result, const XrFovf fov,
                                                    const float nearZ, const float farZ) {
    const float tanLeft = tanf(fov.angleLeft);
    const float tanRight = tanf(fov.angleRight);

    const float tanDown = tanf(fov.angleDown);
    const float tanUp = tanf(fov.angleUp);

    XrMatrix4x4f_CreateProjection(result, tanLeft, tanRight, tanUp, tanDown, nearZ, farZ);
}

class GraphicsOpenGLES {
public:
    struct BufferCreateInfo {
        enum class Type : uint8_t {
            VERTEX,
            INDEX,
            UNIFORM,
        } type;
        size_t stride;
        size_t size;
        void* data;
    };

    enum class VertexType : uint8_t {
        FLOAT,
        VEC2,
        VEC3,
        VEC4,
        INT,
        IVEC2,
        IVEC3,
        IVEC4,
        UINT,
        UVEC2,
        UVEC3,
        UVEC4
    };

    struct ImageCreateInfo {
        uint32_t dimension;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        uint32_t sampleCount;
        int64_t format;
        bool cubemap;
        bool colorAttachment;
        bool depthAttachment;
        bool sampled;
    };

    struct ShaderCreateInfo {
        enum class Type : uint8_t {
            VERTEX,
            TESSELLATION_CONTROL,
            TESSELLATION_EVALUATION,
            GEOMETRY,
            FRAGMENT,
            COMPUTE
        } type;
        const char* sourceData;
        size_t sourceSize;
    };

    struct Viewport {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    enum class SwapchainType : uint8_t {
        COLOR,
        DEPTH
    };
    struct ImageViewCreateInfo {
        void* image;
        enum class Type : uint8_t {
            RTV,
            DSV,
            SRV,
            UAV
        } type;
        enum class View : uint8_t {
            TYPE_1D,
            TYPE_2D,
            TYPE_3D,
            TYPE_CUBE,
            TYPE_1D_ARRAY,
            TYPE_2D_ARRAY,
            TYPE_CUBE_ARRAY,
        } view;
        int64_t format;
        enum class Aspect : uint8_t {
            COLOR_BIT = 0x01,
            DEPTH_BIT = 0x02,
            STENCIL_BIT = 0x04
        } aspect;
        uint32_t baseMipLevel;
        uint32_t levelCount;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

    enum class BlendFactor : uint8_t {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
    };
    enum class BlendOp : uint8_t {
        ADD = 0,
        SUBTRACT = 1,
        REVERSE_SUBTRACT = 2,
        MIN = 3,
        MAX = 4,
    };
    enum class ColorComponentBit : uint8_t {
        R_BIT = 0x00000001,
        G_BIT = 0x00000002,
        B_BIT = 0x00000004,
        A_BIT = 0x00000008,
    };

    struct ColorBlendAttachmentState {
        bool blendEnable;
        BlendFactor srcColorBlendFactor;
        BlendFactor dstColorBlendFactor;
        BlendOp colorBlendOp;
        BlendFactor srcAlphaBlendFactor;
        BlendFactor dstAlphaBlendFactor;
        BlendOp alphaBlendOp;
        ColorComponentBit colorWriteMask;
    };

    enum class LogicOp : uint8_t {
        CLEAR = 0,
        AND = 1,
        AND_REVERSE = 2,
        COPY = 3,
        AND_INVERTED = 4,
        NO_OP = 5,
        XOR = 6,
        OR = 7,
        NOR = 8,
        EQUIVALENT = 9,
        INVERT = 10,
        OR_REVERSE = 11,
        COPY_INVERTED = 12,
        OR_INVERTED = 13,
        NAND = 14,
        SET = 15
    };

    struct VertexInputAttribute {
        uint32_t attribIndex;   // layout(location = X)
        uint32_t bindingIndex;  // Which buffer to use when bound for draws.
        VertexType vertexType;
        size_t offset;
        const char* semanticName;
    };
    typedef std::vector<VertexInputAttribute> VertexInputAttributes;
    struct VertexInputBinding {
        uint32_t bindingIndex;  // Which buffer to use when bound for draws.
        size_t offset;
        size_t stride;
    };

    enum class CompareOp : uint8_t {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7,
    };
    enum class StencilOp : uint8_t {
        KEEP = 0,
        ZERO = 1,
        REPLACE = 2,
        INCREMENT_AND_CLAMP = 3,
        DECREMENT_AND_CLAMP = 4,
        INVERT = 5,
        INCREMENT_AND_WRAP = 6,
        DECREMENT_AND_WRAP = 7
    };
    struct StencilOpState {
        StencilOp failOp;
        StencilOp passOp;
        StencilOp depthFailOp;
        CompareOp compareOp;
        uint32_t compareMask;
        uint32_t writeMask;
        uint32_t reference;
    };
    typedef std::vector<VertexInputBinding> VertexInputBindings;
    struct VertexInputState {
        VertexInputAttributes attributes;
        VertexInputBindings bindings;
    };
    enum class PolygonMode : uint8_t {
        FILL = 0,
        LINE = 1,
        POINT = 2,
    };
    enum class CullMode : uint8_t {
        NONE = 0,
        FRONT = 1,
        BACK = 2,
        FRONT_AND_BACK = 3
    };
    enum class FrontFace : uint8_t {
        COUNTER_CLOCKWISE = 0,
        CLOCKWISE = 1,
    };
    enum class PrimitiveTopology : uint8_t {
        POINT_LIST = 0,
        LINE_LIST = 1,
        LINE_STRIP = 2,
        TRIANGLE_LIST = 3,
        TRIANGLE_STRIP = 4,
        TRIANGLE_FAN = 5,
    };
    struct InputAssemblyState {
        PrimitiveTopology topology;
        bool primitiveRestartEnable;
    };
    struct RasterisationState {
        bool depthClampEnable;
        bool rasteriserDiscardEnable;
        PolygonMode polygonMode;
        CullMode cullMode;
        FrontFace frontFace;
        bool depthBiasEnable;
        float depthBiasConstantFactor;
        float depthBiasClamp;
        float depthBiasSlopeFactor;
        float lineWidth;
    };
    struct MultisampleState {
        uint32_t rasterisationSamples;
        bool sampleShadingEnable;
        float minSampleShading;
        uint32_t sampleMask;
        bool alphaToCoverageEnable;
        bool alphaToOneEnable;
    };
    struct DepthStencilState {
        bool depthTestEnable;
        bool depthWriteEnable;
        CompareOp depthCompareOp;
        bool depthBoundsTestEnable;
        bool stencilTestEnable;
        StencilOpState front;
        StencilOpState back;
        float minDepthBounds;
        float maxDepthBounds;
    };
    struct ColorBlendState {
        bool logicOpEnable;
        LogicOp logicOp;
        std::vector<ColorBlendAttachmentState> attachments;
        float blendConstants[4];
    };
    struct Offset2D {
        int32_t x;
        int32_t y;
    };
    struct Extent2D {
        uint32_t width;
        uint32_t height;
    };
    struct Rect2D {
        Offset2D offset;
        Extent2D extent;
    };

    struct DescriptorInfo {
        uint32_t bindingIndex;
        void* resource;
        enum class Type : uint8_t {
            BUFFER,
            IMAGE,
            SAMPLER
        } type;
        enum class Stage : uint8_t {
            VERTEX,
            TESSELLATION_CONTROL,
            TESSELLATION_EVALUATION,
            GEOMETRY,
            FRAGMENT,
            COMPUTE
        } stage;
        bool readWrite;
        size_t bufferOffset;
        size_t bufferSize;
    };
    struct PipelineCreateInfo {
        std::vector<void*> shaders;
        VertexInputState vertexInputState;
        InputAssemblyState inputAssemblyState;
        RasterisationState rasterisationState;
        MultisampleState multisampleState;
        DepthStencilState depthStencilState;
        ColorBlendState colorBlendState;
        std::vector<int64_t> colorFormats;
        int64_t depthFormat;
        std::vector<DescriptorInfo> layout;
    };

    inline GLenum ToGLCullMode(GraphicsOpenGLES::CullMode cullMode) {
        switch (cullMode) {
            case GraphicsOpenGLES::CullMode::NONE:
                return GL_BACK;
            case GraphicsOpenGLES::CullMode::FRONT:
                return GL_FRONT;
            case GraphicsOpenGLES::CullMode::BACK:
                return GL_BACK;
            case GraphicsOpenGLES::CullMode::FRONT_AND_BACK:
                return GL_FRONT_AND_BACK;
            default:
                return 0;
        }
    }

    inline GLenum ToGLBlendOp(GraphicsOpenGLES::BlendOp op) {
        switch (op) {
            case GraphicsOpenGLES::BlendOp::ADD:
                return GL_FUNC_ADD;
            case GraphicsOpenGLES::BlendOp::SUBTRACT:
                return GL_FUNC_SUBTRACT;
            case GraphicsOpenGLES::BlendOp::REVERSE_SUBTRACT:
                return GL_FUNC_REVERSE_SUBTRACT;
            case GraphicsOpenGLES::BlendOp::MIN:
                return GL_MIN;
            case GraphicsOpenGLES::BlendOp::MAX:
                return GL_MAX;
            default:
                return 0;
        }
    };

    inline GLenum ToGLStencilCompareOp(GraphicsOpenGLES::StencilOp op) {
        switch (op) {
            case GraphicsOpenGLES::StencilOp::KEEP:
                return GL_KEEP;
            case GraphicsOpenGLES::StencilOp::ZERO:
                return GL_ZERO;
            case GraphicsOpenGLES::StencilOp::REPLACE:
                return GL_REPLACE;
            case GraphicsOpenGLES::StencilOp::INCREMENT_AND_CLAMP:
                return GL_INCR;
            case GraphicsOpenGLES::StencilOp::DECREMENT_AND_CLAMP:
                return GL_DECR;
            case GraphicsOpenGLES::StencilOp::INVERT:
                return GL_INVERT;
            case GraphicsOpenGLES::StencilOp::INCREMENT_AND_WRAP:
                return GL_INCR_WRAP;
            case GraphicsOpenGLES::StencilOp::DECREMENT_AND_WRAP:
                return GL_DECR_WRAP;
            default:
                return 0;
        }
    };

    inline GLenum ToGLBlendFactor(GraphicsOpenGLES::BlendFactor factor) {
        switch (factor) {
            case GraphicsOpenGLES::BlendFactor::ZERO:
                return GL_ZERO;
            case GraphicsOpenGLES::BlendFactor::ONE:
                return GL_ONE;
            case GraphicsOpenGLES::BlendFactor::SRC_COLOR:
                return GL_SRC_COLOR;
            case GraphicsOpenGLES::BlendFactor::ONE_MINUS_SRC_COLOR:
                return GL_ONE_MINUS_SRC_COLOR;
            case GraphicsOpenGLES::BlendFactor::DST_COLOR:
                return GL_DST_COLOR;
            case GraphicsOpenGLES::BlendFactor::ONE_MINUS_DST_COLOR:
                return GL_ONE_MINUS_DST_COLOR;
            case GraphicsOpenGLES::BlendFactor::SRC_ALPHA:
                return GL_SRC_ALPHA;
            case GraphicsOpenGLES::BlendFactor::ONE_MINUS_SRC_ALPHA:
                return GL_ONE_MINUS_SRC_ALPHA;
            case GraphicsOpenGLES::BlendFactor::DST_ALPHA:
                return GL_DST_ALPHA;
            case GraphicsOpenGLES::BlendFactor::ONE_MINUS_DST_ALPHA:
                return GL_ONE_MINUS_DST_ALPHA;
            default:
                return 0;
        }
    };

    inline GLenum ToGLTopology(GraphicsOpenGLES::PrimitiveTopology topology) {
        switch (topology) {
            case GraphicsOpenGLES::PrimitiveTopology::POINT_LIST:
                return GL_POINTS;
            case GraphicsOpenGLES::PrimitiveTopology::LINE_LIST:
                return GL_LINES;
            case GraphicsOpenGLES::PrimitiveTopology::LINE_STRIP:
                return GL_LINE_STRIP;
            case GraphicsOpenGLES::PrimitiveTopology::TRIANGLE_LIST:
                return GL_TRIANGLES;
            case GraphicsOpenGLES::PrimitiveTopology::TRIANGLE_STRIP:
                return GL_TRIANGLE_STRIP;
            case GraphicsOpenGLES::PrimitiveTopology::TRIANGLE_FAN:
                return GL_TRIANGLE_FAN;
            default:
                return 0;
        }
    };

    inline GLenum ToGLCompareOp(GraphicsOpenGLES::CompareOp op) {
        switch (op) {
            case GraphicsOpenGLES::CompareOp::NEVER:
                return GL_NEVER;
            case GraphicsOpenGLES::CompareOp::LESS:
                return GL_LESS;
            case GraphicsOpenGLES::CompareOp::EQUAL:
                return GL_EQUAL;
            case GraphicsOpenGLES::CompareOp::LESS_OR_EQUAL:
                return GL_LEQUAL;
            case GraphicsOpenGLES::CompareOp::GREATER:
                return GL_GREATER;
            case GraphicsOpenGLES::CompareOp::NOT_EQUAL:
                return GL_NOTEQUAL;
            case GraphicsOpenGLES::CompareOp::GREATER_OR_EQUAL:
                return GL_GEQUAL;
            case GraphicsOpenGLES::CompareOp::ALWAYS:
                return GL_ALWAYS;
            default:
                return 0;
        }
    };

    GraphicsOpenGLES(XrInstance m_xrInstance, XrSystemId systemId);

    std::vector<int64_t> getSupportedDepthSwapchainFormats();

    void* getSwapchainImage(XrSwapchain swapchain, uint32_t index);

    int64_t selectColorSwapchainFormat(const std::vector<int64_t> &formats);

    void setPipeline(void* pipeline);

    std::vector<int64_t> getSupportedColorSwapchainFormats();

    void* createImageView(const ImageViewCreateInfo &imageViewCI);

    int64_t selectDepthSwapchainFormat(const std::vector<int64_t> &formats);

    GLenum getGLTextureTarget(const ImageCreateInfo &imageCI);

    void setVertexBuffers(void** vertexBuffers, size_t count);

    void setIndexBuffer(void* indexBuffer);

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void setDescriptor(const DescriptorInfo &descriptorInfo);

    void setBufferData(void* buffer, size_t offset, size_t size, void* data);

    void setRenderAttachments(void** colorViews, size_t colorViewCount, void* depthStencilView, uint32_t width, uint32_t height, void* pipeline);

    void* getGraphicsBinding();

    void setViewports(Viewport* viewports, size_t count);

    void setScissors(Rect2D* scissors, size_t count);

    void beginRendering();

    void endRendering();

    void* createPipeline(const PipelineCreateInfo &pipelineCI);

    void clearColor(void* imageView, float r, float g, float b, float a);

    void clearDepth(void* imageView, float d);

    void* createBuffer(const BufferCreateInfo &bufferCI);

    void* createShader(const ShaderCreateInfo &shaderCI);

    XrSwapchainImageBaseHeader* allocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count);

    void destroyImageView(void*& imageView);

    void freeSwapchainImageData(XrSwapchain swapchain);

    void destroyPipeline(void*& pipeline);

    void destroyShader(void*& shader);

    void destroyBuffer(void*& buffer);

public:
    PFN_xrGetOpenGLESGraphicsRequirementsKHR xrGetOpenGLESGraphicsRequirementsKHR = nullptr;
    XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding{};

    std::unordered_map<GLuint, BufferCreateInfo> buffers{};
    std::unordered_map<GLuint, ImageCreateInfo> images{};
    std::unordered_map<GLuint, ImageViewCreateInfo> imageViews{};

    std::unordered_map < XrSwapchain, std::pair<SwapchainType, std::vector<XrSwapchainImageOpenGLESKHR>>> swapchainImagesMap{};

    GLuint setFramebuffer = 0;
    std::unordered_map<GLuint, PipelineCreateInfo> pipelines{};
    GLuint m_setPipeline = 0;
    GLuint vertexArray = 0;
    GLuint m_setIndexBuffer = 0;

    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLConfig eglConfig = nullptr;
    EGLContext eglContext = EGL_NO_CONTEXT;
};