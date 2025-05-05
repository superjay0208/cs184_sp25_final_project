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
    palette[0] = vec3(0.6, 0.4, 1.0); 
    palette[1] = vec3(0.4, 0.6, 1.0); 
    palette[2] = vec3(0.4, 1.0, 1.0); 
    palette[3] = vec3(0.5, 1.0, 0.6); 
    palette[4] = vec3(1.0, 0.9, 0.5); 

    vec3 toneColor;

    if (diff < 0.7) {
    int band;
    if (diff < 0.1) {
        band = int(floor(diff / (0.1 / 5.0)));  
    } else if (diff < 0.3) {
        band = 5 + int(floor((diff - 0.1) / (0.2 / 5.0)));  
    } else {
        band = 10 + int(floor((diff - 0.3) / (0.4 / 5.0))); 
    }
    toneColor = palette[band % 5];
} else {
    int band = int(floor((diff - 0.7) / ((1.0 - 0.7) / 5.0)));  
    toneColor = palette[band % 5];
}

    vec3 baseColor = (ambient + toneColor) * objectColor;

    FragColor = vec4(baseColor, 1.0);
}
