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

    // Define 5-tone repeating palette
    vec3 palette[5];
    palette[4] = vec3(1.0, 0.9, 0.5); // Yellow
    palette[3] = vec3(0.5, 1.0, 0.6); // Green
    palette[2] = vec3(0.4, 1.0, 1.0); // Cyan
    palette[1] = vec3(0.4, 0.6, 1.0); // Blue
    palette[0] = vec3(0.6, 0.4, 1.0); // Purple

    // Increase repetition count to get many bands
    int numBands = 10;
    int toneIndex = int(floor(diff * float(numBands))) % 5;

    vec3 toneColor = palette[toneIndex];
    vec3 baseColor = (ambient + toneColor) * objectColor;

    FragColor = vec4(baseColor, 1.0);
}
