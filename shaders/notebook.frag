#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform float time;

void main()
{
    // --- Sine wave base ---
    float coordY = fs_in.FragPos.y;
    float coordX = fs_in.FragPos.x;

    float frequency = 6.0;
    float speed = 1.0;
    float thickness = 0.1;
    float smoothness = 0.02;

    float wave = sin(coordY * frequency + time * speed);
    wave = 0.5 + 0.5 * wave;

    float waveMask = smoothstep(thickness + smoothness, thickness, abs(wave - 0.5));
    vec3 waveColor = vec3(0.6, 0.75, 1.0); // soft blue

    // --- Vertical red line mask (centered at x=0) ---
    float redLineWidth = 0.02;
    float redLine = smoothstep(redLineWidth + 0.005, redLineWidth, abs(coordX));

    vec3 bgColor = vec3(1.0); // white
    vec3 base = mix(bgColor, waveColor, waveMask);
    base = mix(base, vec3(1.0, 0.0, 0.0), redLine); // blend red line

    FragColor = vec4(base, 1.0);
}
