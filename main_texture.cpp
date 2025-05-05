#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map> 
#include <stdexcept>    
#include <cmath>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h" 

#include "shader.h"     
#include "png.h"      
#include "hw4_helpers/shaderUtils.h" 


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float x_pos = 0.0;
float z_pos = 3.14159265f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 45.0f;

float modelYaw = 0.0f;
float modelPitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float sensitivity = 0.2f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool mouseButtonPressed = false;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords; 
};

struct MeshData {
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
bool loadObjModel(const std::string& filepath, MeshData& meshData); 


int main(int argc, char* argv[]) {
	
	std::string objFilePath = "teapot.obj"; 
	std::string textureFilePath = "default_texture.png"; 
	
	if (argc == 2) {
		objFilePath = argv[1];
		std::cout << "Usage: " << argv[0] << " [path/to/model.obj] [path/to/texture.png]" << std::endl;
		std::cout << "No texture path provided, using default: " << textureFilePath << std::endl;
	} else if (argc >= 3) {
		objFilePath = argv[1];
		textureFilePath = argv[2]; 
	} else {
		std::cout << "Usage: " << argv[0] << " [path/to/model.obj] [path/to/texture.png]" << std::endl;
		std::cout << "No paths provided, using defaults: " << objFilePath << " and " << textureFilePath << std::endl;
	}
	
	
	
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
	
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glEnable(GL_DEPTH_TEST);
	

	Shader toonShader("shaders/crosshatch.vert", "shaders/crosshatch.frag");

	
	GLuint textureID = 0; 
	try {
		textureID = makeTex(textureFilePath.c_str()); 
		if (textureID == 0) {
			throw std::runtime_error("makeTex failed to load texture (check path and PNG format).");
		}
		std::cout << "Loaded texture: " << textureFilePath << " with ID: " << textureID << std::endl;
	} catch (const std::exception& e) { 
		std::cerr << "Error loading texture: " << e.what() << std::endl;
		glfwTerminate();
		return -1;
	}
	
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
	
	
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		

		processInput(window); 
		

		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		toonShader.use();
		
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		toonShader.setMat4("projection", projection);
		
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp); 
		toonShader.setMat4("view", view);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(modelPitch), glm::vec3(1.0f, 0.0f, 0.0f)); 
		model = glm::rotate(model, glm::radians(modelYaw),   glm::vec3(0.0f, 1.0f, 0.0f)); 

		toonShader.setMat4("model", model);
		
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
		toonShader.setMat3("normalMatrix", normalMatrix);
		
		toonShader.setVec3("lightDir", glm::normalize(glm::vec3(0.0f, 1.0f, 0.3f)));
		toonShader.setVec3("lightColor", glm::vec3(1.2f, 1.2f, 1.2f));
		toonShader.setVec3("viewPos", cameraPos);
		
		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, textureID); 
		toonShader.setInt("textureSampler", 0); 
		
		
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		
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
		cameraPos = glm::vec3(50 * std::sin(x_pos), 0.5f, -50 * std::cos(z_pos));
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &textureID);
	
	glfwTerminate();
	return 0;
}


bool loadObjModel(const std::string& filepath, MeshData& meshData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; 
	std::string warn, err;
	
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
		if (!err.empty()) {
			std::cerr << "TinyObjLoader Error: " << err << std::endl;
		}
		return false;
	}
	
	if (!warn.empty()) {
		std::cout << "TinyObjLoader Warning: " << warn << std::endl;
	}
	
	meshData.vertices.clear();
	meshData.indices.clear();
	std::unordered_map<std::string, unsigned int> uniqueVertices{};
	
	for (const auto& shape : shapes) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
			int fv = shape.mesh.num_face_vertices[f];
			if (fv != 3) {
				std::cerr << "Warning: TinyObjLoader found a non-triangle face (vertices=" << fv << "). Skipping." << std::endl;
				index_offset += fv; 
				continue;
			}
			
			
			for (size_t v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
				
				std::string vertexKey = std::to_string(idx.vertex_index) + "/";
				if (idx.normal_index >= 0) vertexKey += std::to_string(idx.normal_index); else vertexKey += "-1";
				if (idx.texcoord_index >= 0) vertexKey += "/" + std::to_string(idx.texcoord_index); else vertexKey += "/-1"; // Add if using texcoords
				if (idx.texcoord_index >= 0) vertexKey += "/" + std::to_string(idx.texcoord_index); else vertexKey += "/-1"; // Add if using texcoords
				
				if (uniqueVertices.count(vertexKey) == 0) {
					Vertex newVertex;
					
					if (idx.vertex_index < 0 || 3 * idx.vertex_index + 2 >= attrib.vertices.size()) {
						throw std::runtime_error("Invalid vertex index in OBJ file.");
					}
					newVertex.Position = {
						attrib.vertices[3 * idx.vertex_index + 0],
						attrib.vertices[3 * idx.vertex_index + 1],
						attrib.vertices[3 * idx.vertex_index + 2]
					};
					
					if (idx.normal_index >= 0 && 3 * idx.normal_index + 2 < attrib.normals.size()) {
						newVertex.Normal = glm::normalize(glm::vec3{ 
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1],
							attrib.normals[3 * idx.normal_index + 2]
						});
					} else {
						std::cerr << "Warning: Missing or invalid normal index for vertex. Using placeholder (0,1,0)." << std::endl;
						newVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); 
					}
					
					if (idx.texcoord_index >= 0 && 2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
						newVertex.TexCoords = {
							attrib.texcoords[2 * idx.texcoord_index + 0],
							attrib.texcoords[2 * idx.texcoord_index + 1]
						};
					} else {
						newVertex.TexCoords = glm::vec2(0.0f, 0.0f);
					}
					
					meshData.vertices.push_back(newVertex);
					uniqueVertices[vertexKey] = static_cast<unsigned int>(meshData.vertices.size() - 1);
				}
				meshData.indices.push_back(uniqueVertices[vertexKey]);
			}
			index_offset += fv;
		}
	}
	
	if (meshData.vertices.empty() || meshData.indices.empty()) {
		std::cerr << "Warning: Loaded OBJ file resulted in empty mesh data." << std::endl;
		return false; 
	}
	
	return true; 
}



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
