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

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 diffuseColor;

    if (diff > 0.9) {
        diffuseColor = vec3(1.0, 1.0, 1.0); 
    } else if (diff > 0.75) {
        diffuseColor = vec3(1.0, 0.6, 0.2); 
    } else if (diff > 0.5) {
        diffuseColor = vec3(1.0, 0.0, 0.0); 
    } else if (diff > 0.3) {
        diffuseColor = vec3(0.4, 0.0, 0.4); 
    } else if (diff > 0.1) {
        diffuseColor = vec3(0.0, 0.0, 0.8); 
    } else {
        diffuseColor = vec3(0.0, 0.0, 0.0); 
    }

    vec3 result = (ambient + diffuseColor) * objectColor;
    FragColor = vec4(result, 1.0);

    /*
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float specularStrength = 0.0;
    if(spec > 0.8) {
       specularStrength = 1.0;
    }
    vec3 specular = specularStrength * lightColor;
    result += specular;
    FragColor = vec4(result, 1.0);
    */
}