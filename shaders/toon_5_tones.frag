#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);

    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;

    // 5-color cyclic palette
    vec3 palette[5];
    palette[0] = vec3(0.6, 0.4, 1.0); // Purple
    palette[1] = vec3(0.4, 0.6, 1.0); // Blue
    palette[2] = vec3(0.4, 1.0, 1.0); // Cyan
    palette[3] = vec3(0.5, 1.0, 0.6); // Green
    palette[4] = vec3(1.0, 0.9, 0.5); // Yellow

    vec3 toneColor;

    if (diff < 0.7) {
    int band;
    if (diff < 0.1) {
        band = int(floor(diff / (0.1 / 5.0)));  // First cycle: 0.00–0.10
    } else if (diff < 0.3) {
        band = 5 + int(floor((diff - 0.1) / (0.2 / 5.0)));  // Second cycle: 0.10–0.30
    } else {
        band = 10 + int(floor((diff - 0.3) / (0.4 / 5.0))); // Third cycle: 0.30–0.70
    }
    toneColor = palette[band % 5];
} else {
    int band = int(floor((diff - 0.7) / ((1.0 - 0.7) / 5.0)));  // Last cycle: 0.70–1.00
    toneColor = palette[band % 5];
}

    vec3 baseColor = (ambient + toneColor) * objectColor;

    FragColor = vec4(baseColor, 1.0);
}
