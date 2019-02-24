#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "vboindexer.hpp"
#include "..\Dependencies\OBJ_loader.h"

#include "opengl_utils.h"

namespace Lab4
{
	class TangentMesh
	{
	public:
		TangentMesh(std::vector<glm::vec3>& _tangents, std::vector<glm::vec3>& _bitangents);
		~TangentMesh();

		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
	};

	class CGObject
	{
	public:
		CGObject();
		~CGObject();
		
		std::vector<objl::Mesh> Meshes;
		std::vector<TangentMesh> tangentMeshes;

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

		void Draw(opengl_utils glutils, GLuint programID);
		glm::mat4 createTransform();
		
		static std::vector<TangentMesh> computeTangentBasis(std::vector<objl::Mesh> Meshes);
		
		static void recalculateVerticesAndIndexes(std::vector<objl::Mesh> Meshes,
			std::vector<objl::Mesh> &new_meshes,
			std::vector<TangentMesh> &new_tangentMeshes);

	private:
		static bool is_near(float v1, float v2);
		static bool getSimilarVertexIndex(
			objl::Vertex & in_vertex,
			std::vector<objl::Vertex> & out_vertices,
			unsigned short & result);
	};
}
