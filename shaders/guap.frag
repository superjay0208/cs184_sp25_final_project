#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDir;

float triangle(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    return step(abs(uv.x) + uv.y, 1.0);
}

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0); // how strongly lit the surface is

    // Control triangle density with light intensity
    float density = mix(20.0, 100.0, diff); // More triangles in brighter areas

    vec2 uv = fs_in.FragPos.xy * density;

    vec2 grid = floor(uv);
    vec2 local = fract(uv);

    // Flip alternate rows for staggered triangles
    if (mod(grid.y, 2.0) > 0.5) {
        local.x = 1.0 - local.x;
    }

    float t = triangle(local);

    // Interpolate green to white based on brightness
    vec3 baseColor = mix(vec3(0.2, 0.9, 0.6), vec3(1.0), diff);

    FragColor = vec4(baseColor * t, 1.0);
}
