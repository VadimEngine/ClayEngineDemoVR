#version 320 es
precision mediump float;

out vec4 FragColor;

uniform vec4 uSolidColor;

void main() {
    FragColor =  uSolidColor;
}