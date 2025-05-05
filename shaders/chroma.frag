#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDirs[6];
uniform int numLights;
uniform vec3 viewPos;

float wave(vec3 pos, float freq, float amp) {
    return sin(pos.x * freq + cos(pos.y * freq)) * amp;
}

void main() {
    vec3 norm = normalize(fs_in.Normal);
    vec3 highlight = vec3(0.0);

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lightDirs[i]);
        float diff = max(dot(norm, lightDir), 0.0);

        float offsetR = wave(fs_in.FragPos, 10.0 + i, 0.02);
        float offsetG = wave(fs_in.FragPos + vec3(5.0), 12.0 + i, 0.02);
        float offsetB = wave(fs_in.FragPos - vec3(5.0), 14.0 + i, 0.02);

        float r = smoothstep(0.7 + offsetR, 1.0, diff);
        float g = smoothstep(0.7 + offsetG, 1.0, diff);
        float b = smoothstep(0.7 + offsetB, 1.0, diff);

        highlight += vec3(r, g, b);
    }

    highlight = clamp(highlight, 0.0, 1.0);


    vec3 baseColor = vec3(0.08, 0.02, 0.12);
    vec3 finalColor = mix(baseColor, vec3(1.0), highlight);

    FragColor = vec4(finalColor, 1.0);
}
