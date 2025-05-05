#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform float time;

void main()
{
    // Parameters
    float frequency = 6.0;        // wave frequency along x
    float amplitude = 0.15;       // vertical distortion amplitude
    float bandSpacing = 5.0;      // vertical repetition density
    float waveSpeed = 1.0;

    // Blue color for lines and gradient
    vec3 waveColor = vec3(0.6, 0.75, 1.0);
    vec3 bgColor = vec3(1.0); // white background

    // Offset Y with sine of X for wavy bands
    float offset = amplitude * sin(fs_in.FragPos.x * frequency + time * waveSpeed);
    float yWithOffset = fs_in.FragPos.y + offset;

    // Compute position within band [0,1]
    float bandPos = fract(yWithOffset * bandSpacing);

    // Create a line at band center (e.g., 0.5)
    float waveLine = smoothstep(0.52, 0.50, bandPos) * smoothstep(0.48, 0.50, bandPos);

    // Create a one-sided gradient *above* the line (closer to 0.0)
    float gradient = smoothstep(0.5, 0.0, bandPos);  // fades from 1  0 going upward

    // Blend: line stays sharp, gradient adds more blue above
    vec3 finalColor = mix(bgColor, waveColor, gradient + waveLine * 0.5);

    FragColor = vec4(finalColor, 1.0);
}
