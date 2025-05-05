#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform float time;

void main()
{

    float frequency = 2.0;        
    float amplitude = 0.15;
    float bandSpacing = 1.5;      
    float waveSpeed = 1.0;

    vec3 waveColor = vec3(0.6, 0.75, 1.0);
    vec3 bgColor = vec3(1.0); 

    float offset = amplitude * sin(fs_in.FragPos.x * frequency + time * waveSpeed);
    float yWithOffset = fs_in.FragPos.y + offset;

    float bandPos = fract(yWithOffset * bandSpacing);

    float waveLine = smoothstep(0.52, 0.50, bandPos) * smoothstep(0.48, 0.50, bandPos);
    float gradient = smoothstep(0.5, 0.0, bandPos);

    vec3 finalColor = mix(bgColor, waveColor, gradient + waveLine * 0.5);
    FragColor = vec4(finalColor, 1.0);
}
