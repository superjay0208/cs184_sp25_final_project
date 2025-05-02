#version 330 core

// Input from vertex shader (interpolated)
in VS_OUT {
    vec3 FragPos;  // Fragment position in world space
    vec3 Normal;   // Fragment normal in world space (interpolated and normalized)
} fs_in;

// Output color for this fragment
out vec4 FragColor;

// Uniforms set from the C++ application
uniform vec3 lightDir;      // Directional light direction (world space, pointing TO light source)
uniform vec3 lightColor;    // Color of the light
uniform vec3 objectColor;   // Base color of the object
uniform vec3 viewPos;       // Camera/viewer position in world space

void main()
{
    // --- Lighting Calculation ---
    // Normalize the incoming normal (interpolation might de-normalize it)
    vec3 norm = normalize(fs_in.Normal);

    // Ensure light direction is normalized
    vec3 lightDirection = normalize(lightDir);

    // Calculate diffuse intensity (Lambertian term)
    // Clamp to 0 to avoid negative light on back faces
    float diff = max(dot(norm, lightDirection), 0.0);

    // --- Toon Shading Steps ---
    // Define a base ambient light level
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Quantize the diffuse intensity into discrete levels
    vec3 diffuseColor;
    if (diff > 0.85) {
        // Brightest level for surfaces directly facing the light
        diffuseColor = lightColor * 1.0;
    } else if (diff > 0.5) {
        // Mid-tone level
        diffuseColor = lightColor * 0.7;
    } else if (diff > 0.15) {
        // Darker tone level
        diffuseColor = lightColor * 0.4;
    } else {
        // Darkest level (almost black) for surfaces facing away
        diffuseColor = vec3(0.0);
    }

    // Combine ambient and quantized diffuse light, modulated by object color
    vec3 result = (ambient + diffuseColor) * objectColor;

    // Set the final fragment color
    FragColor = vec4(result, 1.0);

    // --- Optional: Specular Highlight (Simple Toon Style) ---
    /*
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // Blinn-Phong like calculation

    // Add a sharp specular highlight if intensity is high enough
    float specularStrength = 0.0;
    if(spec > 0.8) { // Sharp threshold for toon specular
       specularStrength = 1.0;
    }
    vec3 specular = specularStrength * lightColor; // Specular is usually light color

    // Add specular to the result (be careful not to oversaturate)
    result += specular;
    FragColor = vec4(result, 1.0);
    */
}

