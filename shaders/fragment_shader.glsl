#version 330 core

in vec2 fragTexCoord;
in float lightIntensity;

uniform sampler2D uTexture; // bound texture
uniform bool uCastsShadows;


out vec4 FragColor;

void main()
{
    vec3 texColor = texture(uTexture, fragTexCoord).rgb;

    if (!uCastsShadows) {
        // If object does NOT cast shadows, just output the texture color
        FragColor = vec4(texColor, 1.0);
        return; // skip further calculations
    }

    // Simple diffuse shading
    vec3 finalColor = texColor * lightIntensity;

    // Optional: add ambient light
    vec3 ambient = 0.5 * texColor;
    finalColor += ambient;

    FragColor = vec4(finalColor, 1.0);
}