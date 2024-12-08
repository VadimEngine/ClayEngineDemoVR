#version 310 es
precision mediump float;

uniform vec4 uColor;
in vec2 TexCoords;

uniform sampler2D theTexture;

out vec4 fragColor;

void main() {
    fragColor = texture(theTexture, TexCoords);
}