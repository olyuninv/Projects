#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>   
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"
#include "lighting.h"

#include "..\Dependencies\OBJ_Loader.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 30
#define NUM_POINT_LIGHTS 2

using namespace glm;
using namespace std;
using namespace Lab6;

// GLFW 
GLFWwindow* window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1100;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


opengl_utils glutils;

bool lbutton_down = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera movement
GLfloat rotate_angle = 0.0f;
bool firstMouse = true;
float myyaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float mypitch = 0.0f;
float lastX = SCR_WIDTH / 2.0; //800.0f / 2.0;
float lastY = SCR_HEIGHT / 2.0; //600.0 / 2.0;
float fov = 45.0f;

// camera
glm::vec3 cameraPos = glm::vec3(2.0f, 4.0f, 2.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Lights
DirectionalLight dirLight = DirectionalLight();
PointLight pointLights[NUM_POINT_LIGHTS];

bool pause = true;

bool useSolidColor = true;
bool useNormalMap = true;
bool useSpecularMap = true;

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

GLuint textures[4 + NUM_POINT_LIGHTS + 1];

unsigned int textureIDcubemap;
unsigned int textureIDlotus;
unsigned int textureIDlotusBump;

unsigned int n_vbovertices = 0;
unsigned int n_ibovertices = 0;
unsigned int n_tangents = 0;
unsigned int n_bitangents = 0;

CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;

// ImGUI definitions
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float increment = 0.1f;
bool shadowSet = false;
bool pointShadowSet = false;
bool dirLightOn = true;
bool light1On = true;
bool light2On = true;

unsigned int sphereIndex = 0;
unsigned int cubeIndex = 0;
unsigned int carpetIndex = 0;
unsigned int teapotIndex = 0;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void addToObjectBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addVBOBufferSubData(VBOindex, cg_object->Meshes[i].Vertices.size(), &cg_object->Meshes[i].Vertices[0].Position.X);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToTangentBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addTBOBufferSubData(VBOindex, cg_object->tangentMeshes[i].tangents.size(), &cg_object->tangentMeshes[i].tangents[0].x);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToBitangentBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addBTBOBufferSubData(VBOindex, cg_object->tangentMeshes[i].bitangents.size(), &cg_object->tangentMeshes[i].bitangents[0].x);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToIndexBuffer(CGObject *cg_object)
{
	int IBOindex = cg_object->startIBO;
	for (auto const& mesh : cg_object->Meshes) {
		glutils.addIBOBufferSubData(IBOindex, mesh.Indices.size(), &mesh.Indices[0]);
		IBOindex += mesh.Indices.size();
	}
}

//---------------------------------------------------------------------------------------------------------//

std::vector<objl::Mesh> loadMeshes(const char* objFileLocation)
{
	objl::Loader obj_loader;

	bool result = obj_loader.LoadFile(objFileLocation);
	if (result && obj_loader.LoadedMeshes.size() > 0)
	{
		return obj_loader.LoadedMeshes;
	}
	else
		throw new exception("Could not load mesh");
}

CGObject loadObjObject(vector<objl::Mesh> meshes, vector<TangentMesh> tangentMeshes, bool addToBuffers, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	CGObject object = CGObject();
	object.Meshes = meshes;
	object.tangentMeshes = tangentMeshes;
	object.subjectToGravity = subjectToGravity;
	object.initialTranslateVector = initTransformVector;
	object.position = initTransformVector;
	object.initialScaleVector = initScaleVector;
	object.color = color;
	object.coef = coef;
	object.Parent = parent;
	object.startVBO = n_vbovertices;
	object.startIBO = n_ibovertices;
	object.VAOs = std::vector<GLuint>();

	if (addToBuffers)
	{
		for (auto const& mesh : meshes) {
			glutils.generateVertexArray(&(VAOs[numVAOs]));
			GLuint tmpVAO = VAOs[numVAOs];
			object.VAOs.push_back(tmpVAO);
			n_vbovertices += mesh.Vertices.size();
			n_ibovertices += mesh.Indices.size();
			numVAOs++;
		}
	}

	return object;
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();

	// LOAD CUBE
	const char* cubeFileName = "../Lab6/meshes/Cube/Cube.obj";
	vector<objl::Mesh> cubeMesh = loadMeshes(cubeFileName);

	std::vector<objl::Mesh> dummy_cubeMeshes = std::vector<objl::Mesh>();
	std::vector<TangentMesh> dummy_cubeTangents = std::vector<TangentMesh>();
	CGObject::recalculateVerticesAndIndexes(cubeMesh, dummy_cubeMeshes, dummy_cubeTangents);

	CGObject cubeObject = loadObjObject(dummy_cubeMeshes, dummy_cubeTangents, true, true, vec3(0.0f, 4.0f, 0.0f), vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numObjects] = cubeObject;
	cubeIndex = numObjects;
	numObjects++;

	// LOAD CYLINDER
	const char* cylinderFileName = "../Lab6/meshes/Cylinder/cylinder_sm.obj";
	vector<objl::Mesh> meshesCylinder = loadMeshes(cylinderFileName);

	std::vector<objl::Mesh> new_meshesCylinder;
	std::vector<TangentMesh> new_tangentMeshesCylinder;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(meshesCylinder, new_meshesCylinder, new_tangentMeshesCylinder);

	CGObject cylinderObject = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, true, true, vec3(-3.0f + 0.1f, 0.0f, -1.8f + 2 * 0.29f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	sceneObjects[numObjects] = cylinderObject;
	numObjects++;

	CGObject cylinderObject2 = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true, vec3(1.0f - 0.1f, 0.0f, -1.8f - 2 * 0.29f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	cylinderObject2.VAOs.push_back(cylinderObject.VAOs[0]);
	cylinderObject2.startVBO = cylinderObject.startVBO;
	cylinderObject2.startIBO = cylinderObject.startIBO;
	sceneObjects[numObjects] = cylinderObject2;
	numObjects++;

	CGObject cylinderObject3 = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true, vec3(3.0f - 0.1f, 0.0f, 1.8f - 2 * 0.29f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	cylinderObject3.VAOs.push_back(cylinderObject.VAOs[0]);
	cylinderObject3.startVBO = cylinderObject.startVBO;
	cylinderObject3.startIBO = cylinderObject.startIBO;
	sceneObjects[numObjects] = cylinderObject3;
	numObjects++;

	CGObject cylinderObject4 = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true, vec3(-1.0f + 0.1f, 0.0f, 1.8f + 2 * 0.29f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	cylinderObject4.VAOs.push_back(cylinderObject.VAOs[0]);
	cylinderObject4.startVBO = cylinderObject.startVBO;
	cylinderObject4.startIBO = cylinderObject.startIBO;
	sceneObjects[numObjects] = cylinderObject4;
	numObjects++;

	// LOAD 
	const char* sphereFileName = "../Lab6/meshes/Sphere/sphere.obj";
	vector<objl::Mesh> sphereMesh = loadMeshes(sphereFileName);

	std::vector<objl::Mesh> sphere_new_meshes;
	std::vector<TangentMesh> sphere_new_tangentMeshes;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(sphereMesh, sphere_new_meshes, sphere_new_tangentMeshes);

	CGObject sphereObject = loadObjObject(sphere_new_meshes, sphere_new_tangentMeshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	sceneObjects[numObjects] = sphereObject;
	sphereIndex = numObjects;
	numObjects++;

	// LOAD PLANE
	const char* planeFileName = "../Lab6/meshes/simplePlane/simplePlane.obj";
	vector<objl::Mesh> meshesPlane = loadMeshes(planeFileName);

	std::vector<objl::Mesh> new_meshesPlane;
	std::vector<TangentMesh> new_tangentMeshesPlane;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(meshesPlane, new_meshesPlane, new_tangentMeshesPlane);

	CGObject planeObject = loadObjObject(new_meshesPlane, new_tangentMeshesPlane, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numObjects] = planeObject;
	carpetIndex = numObjects;
	numObjects++;

	CGObject planeObject2 = loadObjObject(new_meshesPlane, new_tangentMeshesPlane, false, true, vec3(-1.0f, 1.0f, -1.8f), vec3(2.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);
	planeObject2.VAOs.push_back(planeObject.VAOs[0]);
	planeObject2.startVBO = planeObject.startVBO;
	planeObject2.startIBO = planeObject.startIBO;
	planeObject2.initialRotateAngle = vec3(1.57, 0.0, -0.3);
	sceneObjects[numObjects] = planeObject2;
	numObjects++;

	CGObject planeObject3 = loadObjObject(new_meshesPlane, new_tangentMeshesPlane, false, true, vec3(1.0f, 1.0f, 1.8f), vec3(2.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 0.6f), 0.65f, NULL);
	planeObject3.VAOs.push_back(planeObject.VAOs[0]);
	planeObject3.startVBO = planeObject.startVBO;
	planeObject3.startIBO = planeObject.startIBO;
	planeObject3.initialRotateAngle = vec3(1.57, 0.0, -0.3);
	sceneObjects[numObjects] = planeObject3;
	numObjects++;

	// LOAD BENCH
	const char* benchFileName = "../Lab6/meshes/AbstractBench/bench.obj";
	vector<objl::Mesh> meshesBench = loadMeshes(benchFileName);

	std::vector<objl::Mesh> new_meshesBench;
	std::vector<TangentMesh> new_tangentMeshesBench;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(meshesBench, new_meshesBench, new_tangentMeshesBench);

	CGObject benchObject = loadObjObject(new_meshesBench, new_tangentMeshesBench, true, true, vec3(-2.0f, 0.0f, -0.8f), vec3(0.3f, 0.3f, 0.3f), vec3(0.0f, 0.0f, 1.0f), 0.65f, NULL);
	benchObject.initialRotateAngle = vec3(3.14, -0.3, 0.0);
	sceneObjects[numObjects] = benchObject;
	numObjects++;

	// LOAD TEAPOT
	const char* teapotFileName = "../Lab6/meshes/teapot/teapot.obj";
	vector<objl::Mesh> meshesteapot = loadMeshes(teapotFileName);

	std::vector<objl::Mesh> new_meshesteapot;
	std::vector<TangentMesh> new_tangentMeshesteapot;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(meshesteapot, new_meshesteapot, new_tangentMeshesteapot);

	CGObject teapotObject = loadObjObject(new_meshesteapot, new_tangentMeshesteapot, true, true, vec3(0.0f, 0.7f, 1.0f), vec3(0.3f, 0.3f, 0.3f), vec3(1.0f, 0.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numObjects] = teapotObject;
	teapotIndex = numObjects;
	numObjects++;

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);

	addToObjectBuffer(&cubeObject);
	addToObjectBuffer(&cylinderObject);
	addToObjectBuffer(&sphereObject);
	addToObjectBuffer(&planeObject);
	addToObjectBuffer(&benchObject);
	addToObjectBuffer(&teapotObject);

	addToIndexBuffer(&cubeObject);
	addToIndexBuffer(&cylinderObject);
	addToIndexBuffer(&sphereObject);
	addToIndexBuffer(&planeObject);
	addToIndexBuffer(&benchObject);
	addToIndexBuffer(&teapotObject);

	glutils.createTBO(n_vbovertices);
	addToTangentBuffer(&cubeObject);
	addToTangentBuffer(&cylinderObject);
	addToTangentBuffer(&sphereObject);
	addToTangentBuffer(&planeObject);
	addToTangentBuffer(&benchObject);
	addToTangentBuffer(&teapotObject);

	glutils.createBTBO(n_vbovertices);
	addToBitangentBuffer(&cubeObject);
	addToBitangentBuffer(&cylinderObject);
	addToBitangentBuffer(&sphereObject);
	addToBitangentBuffer(&planeObject);
	addToBitangentBuffer(&benchObject);
	addToBitangentBuffer(&teapotObject);

	// Create Quad in a separate buffer
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	glGenVertexArrays(1, &glutils.quadVAO);
	glGenBuffers(1, &glutils.quadVBO);
	glBindVertexArray(glutils.quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void loadCube()
{
	vector<std::string> faces = vector<std::string>();
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_lf.tga");
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_rt.tga");
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_up.tga");
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_dn.tga");
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_ft.tga");
	faces.push_back("../Lab6/meshes/mp_bleak/bleak-outlook_bk.tga");

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
}

void generateTextures()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	loadCube();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glUniform1i(glutils.cubeLocation2, 0);   // cubemap
	glutils.CubeMapShader.setInt("skybox", 0);

	//glUniform1i(glutils.cubeLocation3, 0);   // cubemap
	glutils.ColorShader.setInt("skybox", 0);

	glActiveTexture(GL_TEXTURE1);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	//unsigned char *data = stbi_load("../Lab6/meshes/pear_export_obj/pear_diffuse.jpg", &width, &height, &nrChannels, 3);
	//unsigned char *data = stbi_load("../Lab6/meshes/Carpet/fabric.jpg", &width, &height, &nrChannels, 3);
	unsigned char *data = stbi_load("../Lab6/meshes/grass_park.jpg", &width, &height, &nrChannels, NULL);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data);
	}
	glutils.ColorShader.setInt("diffuseTexture", 1);   // lotus diffuse map

	// Setup lotus texture - bump
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width1, height1, nrChannels1;
	unsigned char *data1 = stbi_load("../Lab6/meshes/pear_export_obj/pear_normal_map.jpg", &width1, &height1, &nrChannels1, 3);
	//unsigned char *data1 = stbi_load("../Lab6/meshes/Carpet/fabric_bump.jpg", &width1, &height1, &nrChannels1, 3);

	if (data1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data1);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data1);
	}
	glutils.ColorShader.setInt("normalTexture", 2);   // lotus diffuse map

	if (useSpecularMap)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textures[3]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load and generate the texture
		int width2, height2, nrChannels2;
		unsigned char *data2 = stbi_load("../Lab6/meshes/pear_export_obj/pear_specular.jpg", &width2, &height2, &nrChannels2, 3);

		if (data2)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data2);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			stbi_image_free(data2);
		}
		glutils.ColorShader.setInt("specularTexture", 3);   // lotus diffuse map
	}
}

void setupLighting()
{
	// directional light	
	dirLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
	dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
	dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);

	pointLights[0].position = glm::vec3(2.2f, 1.5f, 2.4f);
	pointLights[0].color = glm::vec3(1.0, 0.0, 0.0);
	pointLights[0].ambient = glm::vec3(0.05f, 0.05f, 0.05f);
	pointLights[0].diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	pointLights[0].specular = glm::vec3(1.0f, 1.0f, 1.0f);
	pointLights[0].attenuation.constant = 1.0f;
	pointLights[0].attenuation.linear = 0.09f;
	pointLights[0].attenuation.exp = 0.032f;

	pointLights[1].position = glm::vec3(-2.2f, 1.5f, -2.4f);
	pointLights[1].color = glm::vec3(0.0, 1.0, 0.0);
	pointLights[1].ambient = glm::vec3(0.05f, 0.05f, 0.05f);
	pointLights[1].diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	pointLights[1].specular = glm::vec3(1.0f, 1.0f, 1.0f);
	pointLights[1].attenuation.constant = 1.0f;
	pointLights[1].attenuation.linear = 0.09f;
	pointLights[1].attenuation.exp = 0.032f;

	glutils.ColorShader.use();

	glutils.ColorShader.setVec3("dirLight.direction", dirLight.color);
	glutils.ColorShader.setVec3("dirLight.direction", dirLight.direction);
	glutils.ColorShader.setVec3("dirLight.ambient", dirLight.ambient);
	glutils.ColorShader.setVec3("dirLight.diffuse", dirLight.diffuse);
	glutils.ColorShader.setVec3("dirLight.specular", dirLight.specular);

	
}

void createShadowMap_6Faces(int i)
{
	int activeTexture = 4 + i;

	if ( i == 0)
		glActiveTexture(GL_TEXTURE5);
	else
		glActiveTexture(GL_TEXTURE6);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[activeTexture]);

	for (unsigned int j = 0; j < 6; ++j)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void createShadowMap(int i)
{
	int activeTexture = 4 + i;

	glActiveTexture(GL_TEXTURE4);

	glBindTexture(GL_TEXTURE_2D, textures[activeTexture]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	// glDepthFunc(GL_ALWAYS); no depth test
	glEnable(GL_BLEND);// you enable blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	glutils = opengl_utils();

	glutils.createShaders();

	setupLighting();

	glGenTextures(7, textures);  //1, &textureIDcubemap);
	generateTextures();

	createObjects();

	// directional shadow
	glGenFramebuffers(1, &glutils.depthMapFBO[0]);
	createShadowMap(0);
	glBindFramebuffer(GL_FRAMEBUFFER, glutils.depthMapFBO[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[4], 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw "could not attach shadow frame buffer";
	}

	// point shadows
	for (int i = 1; i < NUM_POINT_LIGHTS + 1; i++)
	{
		glGenFramebuffers(1, &glutils.depthMapFBO[i]);
		createShadowMap_6Faces(i);

		glBindFramebuffer(GL_FRAMEBUFFER, glutils.depthMapFBO[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[4 + i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			throw "could not attach shadow frame buffer";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	cout << "Finished loading" << endl;
}

void drawImgui(ImGuiIO& io)
{
	//glfwMakeContextCurrent(window);
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;
	//io.MousePos = ImVec2(0.0, 0.0);
	//io.MouseDragThreshold = 20.0;
	//io.MouseDrawCursor = true;
	//io.WantSetMousePos = true;
	//io.MouseDrawCursor = true;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	//if (show_demo_window)
	//	ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		//static int counter = 0;
		bool imguiBool = true;
		ImGui::Begin("Pre-computed light fields", &imguiBool, ImVec2(576, 680));

		ImGui::Checkbox("Use directional light", &dirLightOn);
		
		ImGui::Text("Directional light shadow map");
		if (shadowSet)
		{
			ImTextureID imTexture = (void*)(intptr_t)textures[4];
			ImGui::Image(imTexture, ImVec2(200, 200));
		}

		ImGui::Separator;
		ImGui::Checkbox("Light 1 On", &light1On);
		ImGui::Text("Position light 1:");
		ImGui::DragFloat3("Light1", &pointLights[0].position.x, increment, -5.0f, 5.0f, "%.1f");
		ImGui::Text("Light1 shadow map");
		if (pointShadowSet)
		{
		/*	ImTextureID imTexture = (void*)(intptr_t)textures[5];
			ImGui::Image(imTexture, ImVec2(200, 200));*/
		}

		ImGui::Separator;

		ImGui::Checkbox("Light 2 On", &light2On);
		ImGui::Text("Position light 2:");
		ImGui::DragFloat3("Light2", &pointLights[1].position.x, increment, -5.0f, 5.0f, "%.1f");
		ImGui::Text("Light2 shadow map");
		if (pointShadowSet)
		{
			/*ImTextureID imTexture = (void*)(intptr_t)textures[6];
			ImGui::Image(imTexture, ImVec2(200, 200));*/
		}

		ImGui::End();
	}

	// Rendering
	int display_w, display_h;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}


void displayCubeMap(glm::mat4 projection, glm::mat4 view)
{
	// First Draw cube map - sceneObjects[0]
	glDepthMask(GL_FALSE);
	glutils.CubeMapShader.use();

	glm::mat4 viewCube = glm::mat4(glm::mat3(view));
	glutils.updateUniformVariablesCubeMap(viewCube, projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	glBindVertexArray(VAOs[0]);

	glutils.bindVertexAttribute(glutils.loc1, 3, sceneObjects[0].startVBO, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glutils.IBO);

	glDrawElements(GL_TRIANGLES, sceneObjects[0].Meshes[0].Indices.size(), GL_UNSIGNED_INT, (void*)(sceneObjects[0].startIBO * sizeof(unsigned int)));

	glDepthMask(GL_TRUE);

}

void displayScene(GLuint shaderId, mat4 view)
{
	// DRAW objects
	for (int i = 0; i < numObjects; i++)
	{
		// Do not draw sphere - this is only used for light shader
		if (i == sphereIndex)
			continue;

		// Do not draw cube for direct shadow
		if (shaderId == glutils.DepthShader.ID && i == cubeIndex || shaderId == glutils.DepthShaderPointLights.ID && i == cubeIndex)
		{
			continue;
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();

		if (shaderId == glutils.DepthShader.ID)
		{
			glutils.updateUniformVariablesShadows(globalCGObjectTransform);
		}
		else if (shaderId == glutils.DepthShaderPointLights.ID)
		{
			glutils.updateUniformVariablesPointShadows(globalCGObjectTransform);
		}
		else
		{
			glutils.updateUniformVariablesReflectance(globalCGObjectTransform, view);
		}

		sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		if (glutils.ColorShader.ID == shaderId)
		{
			glutils.ColorShader.setInt("useSolidColor", useSolidColor);
			glutils.ColorShader.setInt("useNormalMap", useNormalMap);
			glutils.ColorShader.setInt("useSpecularMap", useSpecularMap);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]);
			glutils.ColorShader.setInt("skybox", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[1]);
			glutils.ColorShader.setInt("diffuseTexture", 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glutils.ColorShader.setInt("normalTexture", 2);

			if (useSpecularMap)
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, textures[3]);
				glutils.ColorShader.setInt("specularTexture", 3);
			}

			glutils.ColorShader.setVec3("objectColor", sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);

			if (i == carpetIndex)
			{
				glutils.ColorShader.setInt("useSolidColor", false);
			}
			else
			{
				glutils.ColorShader.setInt("useSolidColor", true);
			}
		}

		sceneObjects[i].Draw(glutils, shaderId);
	}
}

void displayLightBox(glm::mat4 projection, glm::mat4 view)
{
	glutils.LightingShader.use();

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{

		glm::mat4 local1(1.0f);
		local1 = glm::translate(local1, pointLights[i].position);
		local1 = glm::scale(local1, vec3(0.3, 0.3, 0.3));
		glm::mat4 global1 = local1;

		glutils.updateUniformVariablesLighting(global1, view, projection);

		sceneObjects[sphereIndex].Draw(glutils, glutils.LightingShader.ID);
	}
}

mat4 renderShadowsDirectionalLight(DirectionalLight light, GLuint framebuffer, float fov, float near, float far)
{
	// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;

	//lightProjection = glm::perspective(fov, (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near, far); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f); //near, far); // Directional light

	lightView = glm::lookAt(-2.0f * light.direction, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

	// render scene from light's point of view
	glutils.DepthShader.use();
	glutils.DepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);

	displayScene(glutils.DepthShader.ID, lightView);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowSet = true;

	return lightSpaceMatrix;
}

void drawDebugShadowTexture(GLuint depthmap)
{
	// TRY DRAW SHADOW TEXTURE
	glutils.DebugDepthShader.use();
	glDisable(GL_DEPTH_TEST);

	if (depthmap == 4)
		glActiveTexture(GL_TEXTURE4);
	else
		glActiveTexture(GL_TEXTURE5);

	glBindTexture(GL_TEXTURE_2D, textures[depthmap]);

	glutils.DebugDepthShader.setInt("depthMap", depthmap);
	glutils.DebugDepthShader.setFloat("near", -5.0f); //0.1f);
	glutils.DebugDepthShader.setFloat("far", 5.0f); //10.0f);

	glutils.linkCurrentBuffertoShaderDebugDepth();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}

void normalDraw(mat4 lightSpaceMatrix, mat4 view, mat4 projection, float far)
{
	// NORMAL SCENE DRAW
	glutils.ColorShader.use();
	glutils.updateUniformVariablesReflectance(glm::mat4(1.0), view, projection);
	glutils.ColorShader.setVec3("viewPos", cameraPos.x, -cameraPos.y, cameraPos.z);
	glutils.ColorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	glutils.ColorShader.setInt("shadowMapDirectional", 4);
	glutils.ColorShader.setFloat("far_plane", far);
	glutils.ColorShader.setBool("dirLightOn", dirLightOn);
	glutils.ColorShader.setBool("light1On", light1On);
	glutils.ColorShader.setBool("light2On", light2On);
	// point lights
	glutils.ColorShader.setVec3("pointLights[0].position", pointLights[0].position);
	glutils.ColorShader.setVec3("pointLights[0].ambient", pointLights[0].ambient);
	glutils.ColorShader.setVec3("pointLights[0].diffuse", pointLights[0].diffuse);
	glutils.ColorShader.setVec3("pointLights[0].specular", pointLights[0].specular);
	glutils.ColorShader.setFloat("pointLights[0].constant", pointLights[0].attenuation.constant);
	glutils.ColorShader.setFloat("pointLights[0].linear", pointLights[0].attenuation.linear);
	glutils.ColorShader.setFloat("pointLights[0].quadratic", pointLights[0].attenuation.exp);

	// point light 2
	glutils.ColorShader.setVec3("pointLights[1].position", pointLights[1].position);
	glutils.ColorShader.setVec3("pointLights[1].ambient", pointLights[1].ambient);
	glutils.ColorShader.setVec3("pointLights[1].diffuse", pointLights[1].diffuse);
	glutils.ColorShader.setVec3("pointLights[1].specular", pointLights[1].specular);
	glutils.ColorShader.setFloat("pointLights[1].constant", pointLights[1].attenuation.constant);
	glutils.ColorShader.setFloat("pointLights[1].linear", pointLights[1].attenuation.linear);
	glutils.ColorShader.setFloat("pointLights[1].quadratic", pointLights[1].attenuation.exp);

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		if (i == 0)
			glActiveTexture(GL_TEXTURE5);
		else
			glActiveTexture(GL_TEXTURE6);

		glBindTexture(GL_TEXTURE_CUBE_MAP, textures[5+i]);
		glutils.ColorShader.setInt("depthMap[" + std::to_string(i) + "]", 5 + i);
	}

	//glutils.ColorShader.setInt("shadowMap[1]", 5);
	displayScene(glutils.ColorShader.ID, view);
}

std::vector<glm::mat4> ConfigureShaderAndMatrices(vec3 lightPosition, float near, float far)
{
	glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near, far); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene

	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProjection *
		glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	return shadowTransforms;
}

void renderShadowsPointLights(PointLight light, GLuint framebuffer, int i, float fov, float near, float far)
{
	// 1. render depth of scene to texture (from light's perspective)
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	int activeTexture = 4 + i;

	if (i == 1)
		glActiveTexture(GL_TEXTURE5);
	else
		glActiveTexture(GL_TEXTURE6);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[activeTexture]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	std::vector<glm::mat4> shadowsTransforms = ConfigureShaderAndMatrices(light.position, near, far);

	glutils.DepthShaderPointLights.use();

	for (unsigned int j = 0; j < 6; ++j)
		glutils.DepthShaderPointLights.setMat4("shadowMatrices[" + std::to_string(j) + "]", shadowsTransforms[j]);

	glutils.DepthShaderPointLights.setVec3("lightPos", light.position);
	glutils.DepthShaderPointLights.setFloat("far_plane", far);

	displayScene(glutils.DepthShaderPointLights.ID, mat4(1));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	pointShadowSet = true;

	//return lightSpaceMatrix;
}

void display(ImGuiIO& io)
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	drawImgui(io);

	// DRAW CUBEMAP
	//displayCubeMap(projection, view);

	//glCullFace(GL_FRONT);
	mat4 lightSpaceMatrix = renderShadowsDirectionalLight(dirLight, glutils.depthMapFBO[0], glm::radians(fov), 0.1f, 100.0f);
	//glCullFace(GL_BACK);

	float near = 0.1f;
	float far = 100.0f;

	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		renderShadowsPointLights(pointLights[i], glutils.depthMapFBO[i + 1], i + 1, glm::radians(fov), near, far);
	}

	//render 
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update projection 
	
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), near, far);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	displayLightBox(projection, view);
	normalDraw(lightSpaceMatrix, view, projection, far);

	sceneObjects[teapotIndex].rotateAngles.x += 0.01;

	//drawDebugShadowTexture(4);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(void) {
	// Initialise GLFW

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Precomputed Light Fields", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//detect key inputs
	//glfwSetKeyCallback(window, keycallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1); // Enable vsync

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	init();

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		display(io);
	}

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		rotate_angle += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		pause = !pause;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		useNormalMap = !useNormalMap;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		useSpecularMap = !useSpecularMap;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (GLFW_PRESS == action)
		{
			lbutton_down = true;
		}
		else if (GLFW_RELEASE == action)
		{
			lbutton_down = false;
			firstMouse = true;
		}
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (lbutton_down)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		myyaw += xoffset;
		mypitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (mypitch > 89.0f)
			mypitch = 89.0f;
		if (mypitch < -89.0f)
			mypitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(myyaw)) * cos(glm::radians(mypitch));
		front.y = sin(glm::radians(mypitch));
		front.z = sin(glm::radians(myyaw)) * cos(glm::radians(mypitch));
		cameraFront = glm::normalize(front);
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
