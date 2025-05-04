#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map> // Needed for vertex de-duplication
#include <stdexcept>     // Needed for exceptions
#include <cmath>

// Define this in exactly one .cpp file before including the header
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h" // Include tinyobjloader header

#include "shader.h" // Include our shader helper class

// --- Globals ---
// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera settings (Static)
float x_pos = 0.0;
float z_pos = 0.0;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 45.0f;

// Model Rotation settings (controlled by mouse)
float modelYaw = 0.0f;
float modelPitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float sensitivity = 0.2f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse state
bool mouseButtonPressed = false;

// --- Mesh Data Structures ---
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    // Add glm::vec2 TexCoords; if needed
};

struct MeshData {
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
};

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
bool loadObjModel(const std::string& filepath, MeshData& meshData); // OBJ Loader


int main(int argc, char* argv[]) {

    // --- Get OBJ File Path ---
    std::string objFilePath = "teapot.obj"; // Default path
    if (argc > 1) {
        objFilePath = argv[1]; // Use path from command line if provided
    }
    else {
        std::cout << "Usage: " << argv[0] << " [path/to/model.obj]" << std::endl;
        std::cout << "No OBJ path provided, using default: " << objFilePath << std::endl;
    }


    // --- GLFW Initialization ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // --- GLFW Window Creation ---
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Toon Shading OBJ Example", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // --- GLEW Initialization ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // --- OpenGL Configuration ---
    glEnable(GL_DEPTH_TEST);

    // --- Shader Compilation ---
    Shader toonShader("shaders/toon.vert", "shaders/toon.frag");

    // --- Load OBJ Mesh Data ---
    MeshData mesh;
    try {
        if (!loadObjModel(objFilePath, mesh)) {
            std::cerr << "Failed to load OBJ model: " << objFilePath << std::endl;
            glfwTerminate();
            return -1;
        }
        std::cout << "Loaded OBJ model '" << objFilePath << "' with "
            << mesh.vertices.size() << " unique vertices and "
            << mesh.indices.size() << " indices." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading OBJ: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }


    // --- Vertex Buffer Object (VBO), Vertex Array Object (VAO), Element Buffer Object (EBO) ---
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        // --- Per-frame time logic ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- Input ---
        processInput(window);

        // --- Rendering ---
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate the toon shader
        toonShader.use();

        // Set up view and projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraFront, cameraUp);
        toonShader.setMat4("projection", projection);
        toonShader.setMat4("view", view);

        // Set up model matrix based on mouse rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(modelPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        // Optional: Scale model if it's too big/small
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        toonShader.setMat4("model", model);

        // Calculate and set the normal matrix
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        toonShader.setMat3("normalMatrix", normalMatrix);

        // Set lighting and object uniforms
        toonShader.setVec3("lightDir", glm::vec3(0.1f, 1.4f, 1.0f));
        toonShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.95f));
        toonShader.setVec3("objectColor", glm::vec3(0.6f, 0.6f, 0.6f)); // Grey object color
        toonShader.setVec3("viewPos", cameraPos);

        // Bind the VAO for the mesh
        glBindVertexArray(VAO);
        // Draw the mesh using indices from the EBO
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        // Unbind VAO
        glBindVertexArray(0);


        // --- Swap Buffers and Poll Events ---
        glfwSwapBuffers(window);
        glfwPollEvents();

        // --- JR's Camera System ---
        
        if (x_pos < 2 * 3.14159265) {
            x_pos += 0.0005f;
        }
        else {
            x_pos = 0;
        }
        if (z_pos < 2 * 3.14159265) {
            z_pos += 0.0005f;
        }
        else {
            z_pos = 0;
        }
        cameraPos = glm::vec3(20 * std::sin(x_pos), 0.5f, -20 * std::cos(z_pos));
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}


// --- OBJ Loader using tinyobjloader ---
bool loadObjModel(const std::string& filepath, MeshData& meshData) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // Not used in this example
    std::string warn, err;

    // Load the OBJ file
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        // Error occurred during loading
        if (!err.empty()) {
            std::cerr << "TinyObjLoader Error: " << err << std::endl;
        }
        return false;
    }

    // Optional: Print warnings
    if (!warn.empty()) {
        std::cout << "TinyObjLoader Warning: " << warn << std::endl;
    }

    meshData.vertices.clear();
    meshData.indices.clear();
    std::unordered_map<std::string, unsigned int> uniqueVertices{};

    // Iterate over shapes
    for (const auto& shape : shapes) {
        // Iterate over faces (triangles)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];
            if (fv != 3) {
                std::cerr << "Warning: TinyObjLoader found a non-triangle face (vertices=" << fv << "). Skipping." << std::endl;
                index_offset += fv; // Skip this face's vertices
                continue;
            }


            // Iterate over vertices in the face
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // Create a unique key: "vertex_idx/normal_idx/texcoord_idx"
                std::string vertexKey = std::to_string(idx.vertex_index) + "/";
                if (idx.normal_index >= 0) vertexKey += std::to_string(idx.normal_index); else vertexKey += "-1";
                // if (idx.texcoord_index >= 0) vertexKey += "/" + std::to_string(idx.texcoord_index); else vertexKey += "/-1"; // Add if using texcoords

                // Check if this unique vertex combination already exists
                if (uniqueVertices.count(vertexKey) == 0) {
                    // Create a new Vertex
                    Vertex newVertex;

                    // Position (must exist)
                    if (idx.vertex_index < 0 || 3 * idx.vertex_index + 2 >= attrib.vertices.size()) {
                        throw std::runtime_error("Invalid vertex index in OBJ file.");
                    }
                    newVertex.Position = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    // Normal (check existence)
                    if (idx.normal_index >= 0 && 3 * idx.normal_index + 2 < attrib.normals.size()) {
                        newVertex.Normal = {
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        };
                    }
                    else {
                        // Handle missing normals - generate placeholder or throw error
                        std::cerr << "Warning: Missing or invalid normal index for vertex. Using placeholder (0,1,0)." << std::endl;
                        newVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }

                    // Texture Coordinates (check existence if needed)
                    // if (idx.texcoord_index >= 0 && 2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
                    //     newVertex.TexCoords = {
                    //         attrib.texcoords[2 * idx.texcoord_index + 0],
                    //         attrib.texcoords[2 * idx.texcoord_index + 1]
                    //     };
                    // } else {
                    //     newVertex.TexCoords = glm::vec2(0.0f, 0.0f);
                    // }

                    // Add the unique vertex to our list and map
                    meshData.vertices.push_back(newVertex);
                    uniqueVertices[vertexKey] = static_cast<unsigned int>(meshData.vertices.size() - 1);
                }
                // Add the index of the unique vertex to our final index list
                meshData.indices.push_back(uniqueVertices[vertexKey]);
            }
            index_offset += fv;
        }
    }

    if (meshData.vertices.empty() || meshData.indices.empty()) {
        std::cerr << "Warning: Loaded OBJ file resulted in empty mesh data." << std::endl;
        return false; // Or handle appropriately
    }

    return true; // Success
}


// --- GLFW Callback Functions ---

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseButtonPressed = true;
            firstMouse = true;
        }
        else if (action == GLFW_RELEASE) {
            mouseButtonPressed = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!mouseButtonPressed) {
        lastX = static_cast<float>(xposIn);
        lastY = static_cast<float>(yposIn);
        firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    modelYaw += xoffset;
    modelPitch += yoffset;

    if (modelPitch > 89.0f) modelPitch = 89.0f;
    if (modelPitch < -89.0f) modelPitch = -89.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 60.0f) fov = 60.0f;
}
