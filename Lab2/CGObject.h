#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "..\Dependencies\OBJ_loader.h"

#include "opengl_utils.h"

namespace Lab2
{
	class CGObject
	{
	public:
		CGObject();
		~CGObject();

		std::vector<objl::Mesh> Meshes;
		std::vector<GLuint> VAOs;

		// OpenGL 
		int startVBO = 0;
		int startIBO = 0;
		
		// Initial transform
		glm::vec3 initialTranslateVector = glm::vec3(0, 0, 0);
		glm::vec3 initialScaleVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 initialRotateAngle = glm::vec3(0, 0, 0);

		// Transform vectors
		glm::vec3 position = glm::vec3(0, 0, 0);
		glm::vec3 scaleVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rotateAngles = glm::vec3(0, 0, 0);

		// Color
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // White

		// Physics
		bool subjectToGravity = false;
		glm::vec4 velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);   // not moving
		float mass = 1.0f;  // 1kg ?
		float coef = 0.8f;  // coefficient of restitution

		// Current position
		glm::mat4 globalTransform = glm::mat4(1.0);
		
		CGObject *Parent = nullptr;

		void Draw(opengl_utils glutils);
		glm::mat4 createTransform();
	};
}
