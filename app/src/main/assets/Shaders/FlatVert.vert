#version 320 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140, binding = 0) uniform ViewProj {
    mat4 uView;
    mat4 uProj;
};
uniform mat4 uModel;

out vec3 FragPos;
out vec3 FaceNormal;

void main() {
    // Calculate world position of the vertex
    FragPos = vec3(uModel * vec4(aPos, 1.0));

    // Transform normal to world space (but don't normalize here)
    FaceNormal = mat3(transpose(inverse(uModel))) * aNormal;

    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0f);
}