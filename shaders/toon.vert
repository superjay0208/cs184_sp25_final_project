#version 330 core

// Input vertex data: position and normal
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Outputs to the fragment shader (in world space)
out VS_OUT {
    vec3 FragPos;  // Vertex position in world space
    vec3 Normal;   // Vertex normal in world space
} vs_out;

// Uniform matrices for coordinate transformations
uniform mat4 model;      // Model matrix (object -> world)
uniform mat4 view;       // View matrix (world -> view/camera)
uniform mat4 projection; // Projection matrix (view -> clip space)

// Normal matrix (inverse transpose of model matrix) to correctly transform normals
uniform mat3 normalMatrix;

void main()
{
    // Calculate vertex position in world space
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(worldPos);

    // Transform normal to world space using the normal matrix
    // Ensure the normal remains normalized after transformation
    vs_out.Normal = normalize(normalMatrix * aNormal);

    // Calculate final clip space position
    gl_Position = projection * view * worldPos;
}
