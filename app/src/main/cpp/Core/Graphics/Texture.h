#pragma once
// standard lib
// third party
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include <glm/vec2.hpp>
// project
#include "XR/XRUtils.h"

class Texture {
public:
    Texture(const unsigned char* textureData, int width, int height, int channels);

    /** Get the GL Texture Id */
    unsigned int getId() const;

    /** Get Texture width in pixels */
    unsigned int getWidth() const;

    /** Get Texture height in pixels */
    unsigned int getHeight() const;

    /** Get number of channels per pixel */
    unsigned int getChannels() const;

    /** Get Width x Height in pixels */
    glm::ivec2 getShape() const;

private:
    static unsigned int genGLTexture(const unsigned char* textureData, int width, int height, int channels);

    /** GL Texture Id*/
    unsigned int mTextureId_;
    /** Width in pixels*/
    int mWidth_;
    /** Height in pixels*/
    int mHeight_;
    /** Channels per pixel */
    int mChannels_;
};

