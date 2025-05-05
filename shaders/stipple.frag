#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDir;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0); 

    float density = mix(10.0, 100.0, diff);
    vec2 uv = fs_in.FragPos.xy * density;

    vec2 cell = floor(uv);
    vec2 local = fract(uv);

    float r = rand(cell);
    vec2 jitter = vec2(rand(cell + 0.1), rand(cell + 0.2)) * 0.8;

    float d = distance(local, jitter);

    float dotSize = mix(0.05, 0.02, diff); 

    float mask = smoothstep(dotSize, dotSize * 0.8, d);

    FragColor = vec4(vec3(mask), 1.0); 
}
