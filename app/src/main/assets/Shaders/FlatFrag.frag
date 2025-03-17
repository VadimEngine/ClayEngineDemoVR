#version 320 es
precision mediump float;

uniform vec4 uColor;

in vec3 FragPos;
in vec3 FaceNormal;

out vec4 fragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));

    vec3 normal = normalize(FaceNormal);

    // Simple Lambertian shading (diffuse only)
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 color = uColor.rgb * diff + uColor.rgb * 0.2; // Adding some ambient light

    fragColor = vec4(color, 1.0);
}