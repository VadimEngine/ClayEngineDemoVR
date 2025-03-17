#version 320 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(std140, binding = 0) uniform ViewProj {
    mat4 uView;
    mat4 uProj;
};
uniform mat4 uModel;
uniform mat4 uScaleMat;

void main() {
    gl_Position = uProj * uView * uModel * uScaleMat * vec4(aPos, 1.0f);
}