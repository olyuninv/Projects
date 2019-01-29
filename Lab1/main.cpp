#define _CRT_SECURE_NO_WARNINGS

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
#define MAX_OBJECTS 50

using namespace glm;
using namespace std;
using namespace Lab1;

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
glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 30.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool pause = true;

//lighting position
glm::vec3 lightPos(5.0f, 5.0f, 3.0f);

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

int n_vbovertices = 0;
int n_ibovertices = 0;

CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;

void addToObjectBuffer(Lab1::CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addVBOBufferSubData(VBOindex, cg_object->Meshes[i].Vertices.size(), &cg_object->Meshes[i].Vertices[0].Position.X);
		glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToIndexBuffer(Lab1::CGObject *cg_object)
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

Lab1::CGObject loadObjObject(vector<objl::Mesh> meshes, bool addToBuffers, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	Lab1::CGObject object = Lab1::CGObject();
	object.Meshes = meshes;	
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
		for (auto const& mesh :meshes) {					
			glutils.generateVertexArray(&(VAOs[numVAOs]));			
			object.VAOs.push_back(VAOs[numVAOs]);
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
	glutils.getAttributeLocationsPhong();
	glutils.getAttributeLocationsCookTorrence();
	
	const char* boyFileName = "../Lab1/meshes/Boy/boy.obj";
	vector<objl::Mesh> meshes = loadMeshes(boyFileName);   
	CGObject boyObject = loadObjObject(meshes, true, true, vec3(0.0f, 4.0f, 0.0f), vec3(0.08f, 0.08f, 0.08f), vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);  //vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	sceneObjects[numObjects] = boyObject;
	numObjects++;

	//const char* pearsFileName = "../Lab1/meshes/Pears/pear_export.obj";
	//vector<objl::Mesh> meshesPears = loadMeshes(pearsFileName);
	//CGObject pears = loadObjObject(meshesPears, true, true, vec3(0.0f, -3.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(102.0f/255, 204.0f/255, 0.0f), 0.65f, NULL);  //vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	//sceneObjects[numObjects] = pears;
	//numObjects++;

//	const char* motoFileName = "../Lab1/meshes/bottlecap/bottlecap.obj";
	const char* motoFileName = "../Lab1/meshes/moto2/moto2.obj";
	vector<objl::Mesh> meshesMoto = loadMeshes(motoFileName);
	CGObject motoObject = loadObjObject(meshesMoto, true, true, vec3(0.0f, -0.5f, 0.0f), vec3(2.0f, 2.0f, 2.0f), vec3(136.0f/255, 136.0f/255, 136.0f/255), 0.65f, NULL);  //vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	motoObject.initialRotateAngle = vec3(0.0f, 0.0f, 0.3f);
	sceneObjects[numObjects] = motoObject;
	numObjects++;

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);
	
	addToObjectBuffer(&boyObject);
	addToObjectBuffer(&motoObject);
	addToIndexBuffer(&boyObject);
	addToIndexBuffer(&motoObject);
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	glutils = opengl_utils();

	glutils.createShaders();
	
	glutils.setupUniformVariables();

	createObjects();
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

	glPushMatrix();

	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.BlinnPhongID);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH / 3) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesBlinnPhong(global1, view, projection);
	glUniform3f(glutils.lightColorLoc2, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc2, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc2, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef2, 0.1f);
	glUniform1f(glutils.diffuseCoef2, 1.0f);
	glUniform1f(glutils.specularCoef2, 0.5f);
	glUniform1i(glutils.shininess2, 32);

	glViewport(0, 0, SCR_WIDTH / 3, SCR_HEIGHT);
		
	// DRAW 1st object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef2, 0.8f);
			glUniform1i(glutils.shininess2, 128);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesBlinnPhong(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc2, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glLoadIdentity();

	// DRAW 2nd object
	// activate shader
	glUseProgram(glutils.GoochID);
	glutils.updateUniformVariablesGooch(global1, view, projection);
	glUniform3f(glutils.lightPosLoc3, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.coolColor, 159.0f / 255, 148.0f / 255, 255.0f / 255);
	glUniform3f(glutils.warmColor, 255.0f / 255, 75.0f / 255, 75.0f / 255);

	glViewport(SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesGooch(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		//glUniform3f(glutils.objectColorLoc3, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}
	
	glLoadIdentity();

	// DRAW 3rd object
	// activate shader
	glUseProgram(glutils.CookTorrenceID);
	glutils.updateUniformVariablesCookTorrence(global1, view, projection);
	glUniform3f(glutils.lightColorLoc4, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc4, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc4, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef4, 0.1f);
	glUniform1f(glutils.diffuseCoef4, 1.0f);
	glUniform1f(glutils.specularCoef4, 0.5f);
	//glUniform1i(glutils.shininess4, 32);
	glUniform1f(glutils.metallic, 0.3f);
	glUniform1f(glutils.roughness, 0.3f);
	
	glViewport(2* SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			//glUniform1i(glutils.shininess4, 128);
			glUniform1f(glutils.metallic, 0.9f);
			glUniform1f(glutils.roughness, 0.1f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, true);
	}
	
	// rotate
	if (!pause)
	{
		sceneObjects[0].rotateAngles.y += 0.01;
		sceneObjects[1].rotateAngles.y += 0.01;
	}

	glPopMatrix();
	
	// disable VAO
	for (auto const& vao : VAOs) {
		glDisableVertexAttribArray(vao);
	}

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void display2()
{
	// Compare shininess

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.BlinnPhongID);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH / 3) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesBlinnPhong(global1, view, projection);
	glUniform3f(glutils.lightColorLoc2, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc2, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc2, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef2, 0.1f);
	glUniform1f(glutils.diffuseCoef2, 1.0f);
	glUniform1f(glutils.specularCoef2, 0.5f);
			
	//sceneObjects[0].position = vec3(0.0, 0.0, 0.0);

	glViewport(0, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1i(glutils.shininess2, 16);

	// DRAW 1st object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef2, 0.8f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesBlinnPhong(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc2, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glViewport(SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef2, 0.5f);
	glUniform1i(glutils.shininess2, 128);

	// DRAW 2nd object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef2, 0.8f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesBlinnPhong(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc2, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	// DRAW 3rd object
	glViewport(2 * SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef2, 0.5f);
	glUniform1i(glutils.shininess2, 256);
		
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef2, 0.8f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesBlinnPhong(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc2, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}
	
	// rotate
	if (!pause)
	{
		sceneObjects[0].rotateAngles.y += 0.01;
		sceneObjects[1].rotateAngles.y += 0.01;
	}

	glPopMatrix();

	// disable VAO
	for (auto const& vao : VAOs) {
		glDisableVertexAttribArray(vao);
	}

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void display3()
{
	// Compare shininess

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.CookTorrenceID);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH / 3) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesCookTorrence(global1, view, projection);
	
	glUniform3f(glutils.lightColorLoc4, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc4, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc4, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef4, 0.1f);
	glUniform1f(glutils.diffuseCoef4, 1.0f);
	glUniform1f(glutils.specularCoef4, 0.5f);

	glUniform1f(glutils.roughness, 0.3f);

	glViewport(0, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.metallic, 0.1f);

	// DRAW 1st object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);			
			glUniform1f(glutils.roughness, 0.1f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glViewport(SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef4, 0.5f);
	glUniform1f(glutils.roughness, 0.3f);
	glUniform1f(glutils.metallic, 0.5f);

	// DRAW 2nd object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			glUniform1f(glutils.roughness, 0.1f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glViewport(2* SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef4, 0.5f);
	glUniform1f(glutils.roughness, 0.3f);
	glUniform1f(glutils.metallic, 0.9f);

	// DRAW 3rd object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			glUniform1f(glutils.roughness, 0.1f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	// rotate
	if (!pause)
	{
		sceneObjects[0].rotateAngles.y += 0.01;
		sceneObjects[1].rotateAngles.y += 0.01;
	}

	glPopMatrix();

	// disable VAO
	for (auto const& vao : VAOs) {
		glDisableVertexAttribArray(vao);
	}

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void display4()
{
	// Compare shininess

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.CookTorrenceID);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH / 3) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesCookTorrence(global1, view, projection);

	glUniform3f(glutils.lightColorLoc4, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc4, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc4, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef4, 0.1f);
	glUniform1f(glutils.diffuseCoef4, 1.0f);
	glUniform1f(glutils.specularCoef4, 0.5f);
	
	glViewport(0, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.metallic, 0.3f);
	glUniform1f(glutils.roughness, 0.1f);

	// DRAW 1st object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			glUniform1f(glutils.metallic, 0.9f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glViewport(SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef4, 0.5f);
	glUniform1f(glutils.metallic, 0.3f);
	glUniform1f(glutils.roughness, 0.5f);

	// DRAW 2nd object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			glUniform1f(glutils.metallic, 0.9f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	glViewport(2 * SCR_WIDTH / 3, 0, SCR_WIDTH / 3, SCR_HEIGHT);
	glUniform1f(glutils.specularCoef4, 0.5f);
	glUniform1f(glutils.metallic, 0.3f);
	glUniform1f(glutils.roughness, 0.9f);

	// DRAW 3rd object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef4, 0.8f);
			glUniform1f(glutils.metallic, 0.9f);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesCookTorrence(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc4, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}

	// rotate
	if (!pause)
	{
		sceneObjects[0].rotateAngles.y += 0.01;
		sceneObjects[1].rotateAngles.y += 0.01;
	}

	glPopMatrix();

	// disable VAO
	for (auto const& vao : VAOs) {
		glDisableVertexAttribArray(vao);
	}

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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "BRDFs", NULL, NULL);
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
		display4();
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
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		pause = !pause;
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
