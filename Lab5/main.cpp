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

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"

#include "..\Dependencies\OBJ_Loader.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 30

using namespace glm;
using namespace std;
using namespace Lab5;

// GLFW 
GLFWwindow* window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1100;

opengl_utils glutils;

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
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool pause = true;
bool useNormalMap = false;
bool useSpecularMap = false;

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

GLuint textures[4];

unsigned int textureIDcubemap;
unsigned int textureIDlotus;
unsigned int textureIDlotusBump;

unsigned int n_vbovertices = 0;
unsigned int n_ibovertices = 0;
//unsigned int n_tangents = 0;
//unsigned int n_bitangents = 0;

CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;

//lighting position
glm::vec3 lightPos(0.0f, 2.0f, -3.0f);

enum class textureInterpolation
{
	nearest = 1, 
	linear = 2,	
	nearest_mipmap_nearest_interpolation = 3, //GL_NEAREST_MIPMAP_NEAREST: takes the nearest mipmap to match the pixel size and uses nearest neighbor interpolation for texture sampling.
	nearest_mipmap_linear_interpolation = 4, //GL_LINEAR_MIPMAP_NEAREST : takes the nearest mipmap level and samples using linear interpolation.
	interpolate_mipmap_nearest_interpolation = 5, //GL_NEAREST_MIPMAP_LINEAR : linearly interpolates between the two mipmaps that most closely match the size of a pixel and samples via nearest neighbor interpolation.
	interpolate_mipmap_linear_interpolation = 6, //GL_LINEAR_MIPMAP_LINEAR : linearly interpolates between the two closest mipmaps and samples the texture via linear interpolation.* /	
	
};

textureInterpolation texInterpolationType = textureInterpolation::nearest;

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

CGObject createChessboard(int &numberOfObjects)
{
	const char* chessboardFileName = "../Lab5/meshes/SimplePlane/simplePlane.obj";
	vector<objl::Mesh> meshesChessboard = loadMeshes(chessboardFileName);

	std::vector<objl::Mesh> new_meshesChessboard;
	std::vector<TangentMesh> new_tangentMeshesChessboard;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(meshesChessboard, new_meshesChessboard, new_tangentMeshesChessboard);

	CGObject chessboardObject = loadObjObject(new_meshesChessboard, new_tangentMeshesChessboard, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);  // vec3(160/ 255.0, 82/255.0, 45/255.0) - brown  //vec3(173/255.0, 255/255.0, 47/255.0) - green
	sceneObjects[numberOfObjects] = chessboardObject;
	numberOfObjects++;

	return chessboardObject;
}


CGObject createCubeMap(int &numberOfObjects)
{
	// CUBEMAP
	const char* cubeFileName = "../Lab5/meshes/Cube/Cube.obj";
	vector<objl::Mesh> cubeMesh = loadMeshes(cubeFileName);

	std::vector<objl::Mesh> dummy_cubeMeshes = std::vector<objl::Mesh>();
	std::vector<TangentMesh> dummy_cubeTangents = std::vector<TangentMesh>();
	CGObject::recalculateVerticesAndIndexes(cubeMesh, dummy_cubeMeshes, dummy_cubeTangents);

	CGObject cubeObject = loadObjObject(dummy_cubeMeshes, dummy_cubeTangents, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numberOfObjects] = cubeObject;
	numberOfObjects++;

	return cubeObject;
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();

	CGObject cubeObject = createCubeMap(numObjects);

	CGObject chessboardObject = createChessboard(numObjects);

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);

	addToObjectBuffer(&cubeObject);
	addToObjectBuffer(&chessboardObject);
	addToIndexBuffer(&cubeObject);
	addToIndexBuffer(&chessboardObject);

	glutils.createTBO(n_vbovertices);
	addToTangentBuffer(&cubeObject);
	addToTangentBuffer(&chessboardObject);

	glutils.createBTBO(n_vbovertices);
	addToBitangentBuffer(&cubeObject);
	addToBitangentBuffer(&chessboardObject);
}

void loadCube()
{
	vector<std::string> faces = vector<std::string>();
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_lf.tga");
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_rt.tga");
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_up.tga");
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_dn.tga");
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_ft.tga");
	faces.push_back("../Lab5/meshes/mp_bleak/bleak-outlook_bk.tga");

	/*faces.push_back("../Lab3/meshes/mp_awup/awup_lf.tga");
	faces.push_back("../Lab3/meshes/mp_awup/awup_rt.tga");
	faces.push_back("../Lab3/meshes/mp_awup/awup_up.tga");
	faces.push_back("../Lab3/meshes/mp_awup/awup_dn.tga");
	faces.push_back("../Lab3/meshes/mp_awup/awup_ft.tga");
	faces.push_back("../Lab3/meshes/mp_awup/awup_bk.tga");*/

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

void init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);// you enable blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glutils = opengl_utils();

	glutils.createShaders();

	glutils.setupUniformVariables();


	// Setup cubemap texture
	glGenTextures(4, textures);  //1, &textureIDcubemap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	loadCube();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glUniform1i(glutils.cubeLocation2, 0);   // cubemap
	glUniform1i(glutils.cubeLocation3, 0);   // cubemap

	glActiveTexture(GL_TEXTURE1);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/*When switching between mipmaps levels during rendering OpenGL might show some artifacts like sharp edges visible between the two mipmap layers. Just like normal texture filtering, it is also possible to filter between mipmap levels using NEAREST and LINEAR filtering for switching between mipmap levels. To specify the filtering method between mipmap levels we can replace the original filtering methods with one of the following four options:

GL_NEAREST_MIPMAP_NEAREST: takes the nearest mipmap to match the pixel size and uses nearest neighbor interpolation for texture sampling.
GL_LINEAR_MIPMAP_NEAREST: takes the nearest mipmap level and samples using linear interpolation.
GL_NEAREST_MIPMAP_LINEAR: linearly interpolates between the two mipmaps that most closely match the size of a pixel and samples via nearest neighbor interpolation.
GL_LINEAR_MIPMAP_LINEAR: linearly interpolates between the two closest mipmaps and samples the texture via linear interpolation.*/
	   

	switch (texInterpolationType)
	{
	case textureInterpolation::nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case textureInterpolation::linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case textureInterpolation::nearest_mipmap_nearest_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		break;
	case textureInterpolation::nearest_mipmap_linear_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		break;
	case textureInterpolation::interpolate_mipmap_nearest_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		break;
	case textureInterpolation::interpolate_mipmap_linear_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;	
	default:
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load("../Lab5/meshes/Chess_Board/Chess_Board.jpg", &width, &height, &nrChannels, NULL);
	//unsigned char *data = stbi_load("../Lab5/meshes/Chess_Board/BrickWall.jpg", &width, &height, &nrChannels, NULL);
	//unsigned char *data = stbi_load("../Lab5/meshes/Chess_Board/pokemons.jpg", &width, &height, &nrChannels, NULL);
	//unsigned char *data = stbi_load("../Lab5/meshes/Chess_Board/pokemon2_sm.jpg", &width, &height, &nrChannels, NULL);
	
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

	glUniform1i(glutils.texture3, 1);

	if (useNormalMap)
	{
		// Setup lotus texture - bump
		glActiveTexture(GL_TEXTURE2);
		//glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[2]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load and generate the texture
		int width1, height1, nrChannels1;
		unsigned char *data1 = stbi_load("../Lab5/meshes/Chess_Board/Chess_Board.jpg", &width1, &height1, &nrChannels1, 4);

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
		glUniform1i(glutils.normalMap3, 2);
	}

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
		unsigned char *data2 = stbi_load("../Lab5/meshes/simplePlane/600px-Chess_Board.png", &width2, &height2, &nrChannels2, 3);

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
		glUniform1i(glutils.specularMap3, 3);
	}

	createObjects();
}

void displayScene(glm::mat4 projection, glm::mat4 view)
{
	glPushMatrix();

	glLoadIdentity();

	glUseProgram(glutils.ShaderWithTextureID);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesReflectance(global1, view, projection);

	glUniform3f(glutils.viewPosLoc3, cameraPos.x, -cameraPos.y, cameraPos.z);
	glUniform3f(glutils.lightPosLoc3, lightPos.x, lightPos.y, lightPos.z);
	glUniform1i(glutils.useNormalMapUniform3, useNormalMap);
	glUniform1i(glutils.useSpecularMapUniform3, useSpecularMap);

	// DRAW objects
	for (int i = 1; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesReflectance(globalCGObjectTransform, view);
		sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]);
		glUniform1i(glutils.cubeLocation3, 0);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glutils.texture3, 1);

		if (useNormalMap)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glUniform1i(glutils.normalMap3, 2);
		}

		if (useSpecularMap)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textures[3]);
			glUniform1i(glutils.specularMap3, 3);
		}

		/*glUniform3f(glutils.objectColorLoc3, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, glutils.ShaderWithTextureID);*/

		for (int j = 1; j < 10; j++)
		{
			for (int k = -10; k < 10; k++)
			{
				glm::mat4 localTransform = glm::mat4(1.0);
				localTransform = glm::translate(localTransform, vec3(k * (-2.0), 0.0, j * (-2.0)));
				glm::mat4 globalTransform2 = sceneObjects[i].globalTransform * localTransform;
				glutils.updateUniformVariablesReflectance(globalTransform2, view);
				sceneObjects[i].Draw(glutils, glutils.ShaderWithTextureID);
			}
		}

		glDisable(GL_TEXTURE_2D);
	}

	glPopMatrix();

}

void displayCubeMap(glm::mat4 projection, glm::mat4 view)
{
	glPushMatrix();

	glLoadIdentity();

	// First Draw cube map - sceneObjects[0]
	glDepthMask(GL_FALSE);
	glUseProgram(glutils.CubeMapID);

	glm::mat4 viewCube = glm::mat4(glm::mat3(view));
	glutils.updateUniformVariablesCubeMap(viewCube, projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	glBindVertexArray(VAOs[0]);

	glutils.bindVertexAttribute(glutils.loc4, 3, sceneObjects[0].startVBO, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glutils.IBO);

	glDrawElements(GL_TRIANGLES, sceneObjects[0].Meshes[0].Indices.size(), GL_UNSIGNED_INT, (void*)(sceneObjects[0].startIBO * sizeof(unsigned int)));

	glDepthMask(GL_TRUE);

	glPopMatrix();

}

void display()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	switch (texInterpolationType)
	{
	case textureInterpolation::nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case textureInterpolation::linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case textureInterpolation::nearest_mipmap_nearest_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		break;
	case textureInterpolation::nearest_mipmap_linear_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		break;
	case textureInterpolation::interpolate_mipmap_nearest_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		break;
	case textureInterpolation::interpolate_mipmap_linear_interpolation:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	default:
		break;
	}

	// DRAW CUBEMAP
	displayCubeMap(projection, view);

	// DRAW SCENE
	displayScene(projection, view);

	// rotate
	if (!pause)
	{
		sceneObjects[1].rotateAngles.y += 0.01;
		sceneObjects[2].rotateAngles.y += 0.01;
		sceneObjects[3].rotateAngles.y += 0.01;
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(void) {
	// Initialise GLFW
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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Surface Mapping", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//detect key inputs
	//glfwSetKeyCallback(window, keycallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	init();

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		display();
	}

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();

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
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::nearest;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::linear;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::nearest_mipmap_nearest_interpolation;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::nearest_mipmap_linear_interpolation;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::interpolate_mipmap_nearest_interpolation;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		texInterpolationType = textureInterpolation::interpolate_mipmap_linear_interpolation;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
