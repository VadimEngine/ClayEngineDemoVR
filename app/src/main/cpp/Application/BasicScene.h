#pragma once
// third party
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// core
#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Camera.h"

class BasicScene {
public:
    // Vertex Shader
    const char* vertexShaderSource = R"(
        #version 300 es
        layout (location=0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0f);
        }
    )";

    // Fragment Shader
    const char* fragmentShaderSource = R"(
        #version 300 es
        precision mediump float;

        out vec4 fragColor;

        void main() {
            fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // White color
        }
    )";
    static float triangleVertices[];

    GLuint vao;

    Shader* mShader_ = nullptr;
    Camera mCamera_;

    BasicScene();
    ~BasicScene();

    Camera* getCamera();

    void render();
};


