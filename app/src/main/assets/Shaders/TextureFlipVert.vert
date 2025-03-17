#version 320 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 uModel;
layout(std140, binding = 0) uniform ViewProj {
    mat4 uView;
    mat4 uProj;
};

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0f);
}