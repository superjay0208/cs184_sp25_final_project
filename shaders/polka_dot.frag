#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform float time;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

vec3 getRGBColor(float val) {
    if (val < 0.33) return vec3(1.0, 0.2, 0.2);   
    else if (val < 0.66) return vec3(0.2, 1.0, 0.2); 
    else return vec3(0.2, 0.4, 1.0);               
}

float circle(vec2 uv, float radius) {
    return smoothstep(radius, radius - 0.01, length(uv - 0.5));
}

void main() {

    vec2 gridUV = fs_in.FragPos.xy * 3.0;
    vec2 cell = floor(gridUV);
    vec2 localUV = fract(gridUV);

    float h = hash(cell);
    vec3 dotColor = getRGBColor(h);

    float mask = circle(localUV, 0.35);

    vec3 baseColor = vec3(0.08, 0.03, 0.12);

    vec3 finalColor = mix(baseColor, dotColor, mask);
    FragColor = vec4(finalColor, 1.0);
}
