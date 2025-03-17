#version 320 es
precision mediump float;

uniform vec4 uColor;
in vec2 TexCoords;

uniform sampler2D theTexture;

out vec4 fragColor;

void main() {
    vec2 flippedTexCoords = vec2(TexCoords.x, 1.0 - TexCoords.y);
    fragColor = texture(theTexture, flippedTexCoords);
}