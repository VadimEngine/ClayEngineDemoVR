#version 320 es
layout (location = 0) in vec4 vertex;  // <vec2 pos, vec2 tex>

layout(std140, binding = 0) uniform ViewProj {
    mat4 uView;
    mat4 uProj;
};
uniform mat4 uModel;

out vec2 TexCoords;

void main() {
    TexCoords = vertex.zw;
    gl_Position = uProj * uView * uModel * vec4(vertex.xy, 0.0, 1.0);
}