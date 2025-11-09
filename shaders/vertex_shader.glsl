#version 330 core

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 fragPos;    // world-space position
out vec3 fragNormal; // world-space normal
out vec3 fragColor;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    fragPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    fragNormal = normalize(normalMatrix * aNormal);

    fragColor = aColor;

    gl_Position = uProjection * uView * worldPos;
}
