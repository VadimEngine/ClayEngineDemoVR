#version 320 es
precision mediump float;

uniform sampler2D text;
uniform vec3 textColor;

in vec2 TexCoords;
out vec4 fragColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    fragColor = vec4(textColor, 1.0) * sampled;
}