#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

out vec4 FragColor;

uniform vec3 lightDir = normalize(vec3(0.3, 1.0, 0.0)); // directional light
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(normal, normalize(lightDir)), 0.0); // Lambert
    vec3 color = fragColor * diff * lightColor;

    FragColor = vec4(color, 1.0);
}
