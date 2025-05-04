#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform float time;

// Hash function to select color
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Returns color based on index
vec3 getRGBColor(float val) {
    if (val < 0.33) return vec3(1.0, 0.2, 0.2);    // Red
    else if (val < 0.66) return vec3(0.2, 1.0, 0.2); // Green
    else return vec3(0.2, 0.4, 1.0);               // Blue
}

// Circle mask
float circle(vec2 uv, float radius) {
    return smoothstep(radius, radius - 0.01, length(uv - 0.5));
}

void main() {
    // Tile coordinates (increase multiplier for more dots)
    vec2 gridUV = fs_in.FragPos.xy * 3.0;
    vec2 cell = floor(gridUV);
    vec2 localUV = fract(gridUV);

    // Hash to pick color
    float h = hash(cell);
    vec3 dotColor = getRGBColor(h);

    // Dot mask
    float mask = circle(localUV, 0.35);

    // Background (dark violet)
    vec3 baseColor = vec3(0.08, 0.03, 0.12);

    vec3 finalColor = mix(baseColor, dotColor, mask);
    FragColor = vec4(finalColor, 1.0);
}
