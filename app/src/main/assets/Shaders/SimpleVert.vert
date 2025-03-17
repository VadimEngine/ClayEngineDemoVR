#version 320 es
layout (location=0) in vec3 aPos;

layout(std140, binding = 0) uniform ViewProj {
    mat4 uView;
    mat4 uProj;
};
uniform mat4 uModel;

void main() {
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0f);
}