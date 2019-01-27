#pragma once

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace Lab1
{
	class opengl_utils
	{

	public:
		opengl_utils()
		{}

		~opengl_utils()
		{}
		
		// Shader attribute locations
		GLuint loc1;
		GLuint loc2;
		GLuint loc3;
		GLuint loc4;
		GLuint loc5;
		GLuint loc6;

		// Shaders
		GLuint PhongProgramID;
		GLuint BlinnPhongID;
		GLuint GoochID;
		GLuint CookTorrenceID;

		// Buffers
		GLuint VBO;
		GLuint IBO;

		// Uniform locations - Phong	
		GLint model_mat_location;
		GLint view_mat_location;
		GLint proj_mat_location;
		GLint objectColorLoc;
		GLint lightColorLoc;
		GLint lightPosLoc;
		GLint viewPosLoc;
		GLint ambientCoef;
		GLint diffuseCoef;
		GLint specularCoef;
		GLint shininess;

		// Uniform locations - BlinnPhong
		GLint model_mat_location2;
		GLint view_mat_location2;
		GLint proj_mat_location2;
		GLint objectColorLoc2;
		GLint lightColorLoc2;
		GLint lightPosLoc2;
		GLint viewPosLoc2;
		GLint ambientCoef2;
		GLint diffuseCoef2;
		GLint specularCoef2;
		GLint shininess2;

		// Uniform locations - Gooch
		GLint model_mat_location3;
		GLint view_mat_location3;
		GLint proj_mat_location3;
		GLint lightPosLoc3;
		GLint warmColor;
		GLint coolColor;

		// Uniform locations - CookTorrence
		GLint model_mat_location4;
		GLint view_mat_location4;
		GLint proj_mat_location4;
		GLint objectColorLoc4;
		GLint lightColorLoc4;
		GLint lightPosLoc4;
		GLint viewPosLoc4;
		GLint ambientCoef4;
		GLint diffuseCoef4;
		GLint specularCoef4;
		GLint shininess4;

		static GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

			// Create the shaders
			GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

			// Read the Vertex Shader code from the file
			std::string VertexShaderCode;
			std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
			if (VertexShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << VertexShaderStream.rdbuf();
				VertexShaderCode = sstr.str();
				VertexShaderStream.close();
			}
			else {
				printf("impossible to open %s. are you in the right directory ? don't forget to read the faq !\n", vertex_file_path);
				getchar();
				return 0;
			}

			// Read the Fragment Shader code from the file
			std::string FragmentShaderCode;
			std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
			if (FragmentShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << FragmentShaderStream.rdbuf();
				FragmentShaderCode = sstr.str();
				FragmentShaderStream.close();
			}

			GLint Result = GL_FALSE;
			int InfoLogLength;

			// Compile Vertex Shader
			printf("Compiling shader : %s\n", vertex_file_path);
			char const * VertexSourcePointer = VertexShaderCode.c_str();
			glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
			glCompileShader(VertexShaderID);

			// Check Vertex Shader
			glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
				printf("%s\n", &VertexShaderErrorMessage[0]);
			}

			// Compile Fragment Shader
			printf("Compiling shader : %s\n", fragment_file_path);
			char const * FragmentSourcePointer = FragmentShaderCode.c_str();
			glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
			glCompileShader(FragmentShaderID);

			// Check Fragment Shader
			glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
				printf("%s\n", &FragmentShaderErrorMessage[0]);
			}

			// Link the program
			printf("Linking program\n");
			GLuint ProgramID = glCreateProgram();
			glAttachShader(ProgramID, VertexShaderID);
			glAttachShader(ProgramID, FragmentShaderID);
			glLinkProgram(ProgramID);
			printf("Finished linking\n");


			// Check the program
			glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
			glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
				glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s\n", &ProgramErrorMessage[0]);
			}

			glDetachShader(ProgramID, VertexShaderID);
			glDetachShader(ProgramID, FragmentShaderID);

			glDeleteShader(VertexShaderID);
			glDeleteShader(FragmentShaderID);

			return ProgramID;
		}

		void createShaders()
		{
			// Create and compile our shaders
			PhongProgramID = LoadShaders("../Lab1/shaders/phong.vs", "../Lab1/shaders/phong.fs");
			BlinnPhongID = LoadShaders("../Lab1/shaders/blinnPhong.vs", "../Lab1/shaders/blinnPhong.fs");
			GoochID = LoadShaders("../Lab1/shaders/Gooch.vs", "../Lab1/shaders/Gooch.fs");
			CookTorrenceID = LoadShaders("../Lab1/shaders/CookTorrence.vs", "../Lab1/shaders/CookTorrence.fs");
		}

		void createVBO(int numVertices)
		{
			// Create VBO
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), NULL, GL_STATIC_DRAW);  // Vertex contains 8 floats: position (vec3), normal (vec3), texture (vec2)
		}

		void createIBO(int numVertices)
		{
			// Create IBO
			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices * sizeof(unsigned int), NULL, GL_STATIC_DRAW);
		}

		void addVBOBufferSubData(int startIndex, int meshSize, float *meshAddress)
		{
			glBufferSubData(GL_ARRAY_BUFFER, startIndex * 8 * sizeof(GLfloat), meshSize * 8 * sizeof(GLfloat), meshAddress);
		}

		void addIBOBufferSubData(int startIndex, int meshSize, const GLuint *meshAddress)
		{
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, startIndex * sizeof(GLuint), sizeof(GLuint) * meshSize, meshAddress);
		}
		
		void generateVertexArray(GLuint *array)
		{
			glGenVertexArrays(1, array);
		}

		void setupUniformVariables()
		{
			// Phong
			model_mat_location = glGetUniformLocation(PhongProgramID, "model");
			view_mat_location = glGetUniformLocation(PhongProgramID, "view");
			proj_mat_location = glGetUniformLocation(PhongProgramID, "projection");

			objectColorLoc = glGetUniformLocation(PhongProgramID, "objectColor");
			lightColorLoc = glGetUniformLocation(PhongProgramID, "lightColor");
			lightPosLoc = glGetUniformLocation(PhongProgramID, "lightPos");
			viewPosLoc = glGetUniformLocation(PhongProgramID, "viewPos");

			ambientCoef = glGetUniformLocation(PhongProgramID, "ambientCoef");
			diffuseCoef = glGetUniformLocation(PhongProgramID, "diffuseCoef");
			specularCoef = glGetUniformLocation(PhongProgramID, "specularCoef");
			shininess = glGetUniformLocation(PhongProgramID, "shininess");

			// BlinnPhong
			model_mat_location2 = glGetUniformLocation(BlinnPhongID, "model");
			view_mat_location2 = glGetUniformLocation(BlinnPhongID, "view");
			proj_mat_location2 = glGetUniformLocation(BlinnPhongID, "projection");

			objectColorLoc2 = glGetUniformLocation(BlinnPhongID, "objectColor");
			lightColorLoc2 = glGetUniformLocation(BlinnPhongID, "lightColor");
			lightPosLoc2 = glGetUniformLocation(BlinnPhongID, "lightPos");
			viewPosLoc2 = glGetUniformLocation(BlinnPhongID, "viewPos");
			
			ambientCoef2 = glGetUniformLocation(PhongProgramID, "ambientCoef");
			diffuseCoef2 = glGetUniformLocation(PhongProgramID, "diffuseCoef");
			specularCoef2 = glGetUniformLocation(PhongProgramID, "specularCoef");
			shininess2 = glGetUniformLocation(PhongProgramID, "shininess");

			// Gooch
			model_mat_location3 = glGetUniformLocation(GoochID, "model");
			view_mat_location3 = glGetUniformLocation(GoochID, "view");
			proj_mat_location3 = glGetUniformLocation(GoochID, "projection");

			lightPosLoc3 = glGetUniformLocation(GoochID, "lightPos");
			warmColor = glGetUniformLocation(GoochID, "warmColor");
			coolColor = glGetUniformLocation(GoochID, "coolColor");

			// Cook Torrence
			model_mat_location4 = glGetUniformLocation(BlinnPhongID, "model");
			view_mat_location4 = glGetUniformLocation(BlinnPhongID, "view");
			proj_mat_location4 = glGetUniformLocation(BlinnPhongID, "projection");

			objectColorLoc4 = glGetUniformLocation(BlinnPhongID, "objectColor");
			lightColorLoc4 = glGetUniformLocation(BlinnPhongID, "lightColor");
			lightPosLoc4 = glGetUniformLocation(BlinnPhongID, "lightPos");
			viewPosLoc4 = glGetUniformLocation(BlinnPhongID, "viewPos");

			ambientCoef4 = glGetUniformLocation(PhongProgramID, "ambientCoef");
			diffuseCoef4 = glGetUniformLocation(PhongProgramID, "diffuseCoef");
			specularCoef4 = glGetUniformLocation(PhongProgramID, "specularCoef");
			shininess4 = glGetUniformLocation(PhongProgramID, "shininess");
		}

		void getAttributeLocationsPhong()
		{
			loc1 = glGetAttribLocation(PhongProgramID, "position");
			loc2 = glGetAttribLocation(PhongProgramID, "normal");
			loc3 = glGetAttribLocation(PhongProgramID, "texture");
		}

		void getAttributeLocationsCookTorrence()
		{
			loc4 = glGetAttribLocation(CookTorrenceID, "position");
			loc5 = glGetAttribLocation(CookTorrenceID, "normal");
			loc6 = glGetAttribLocation(CookTorrenceID, "texture");
		}

		/*void getAttributeLocationsBlinnPhong()
		{
			loc1 = glGetAttribLocation(BlinnPhongID, "position");
			loc2 = glGetAttribLocation(BlinnPhongID, "normal");
			loc3 = glGetAttribLocation(BlinnPhongID, "texture");
		}*/

		void bindVertexAttribute(int location, int locationSize, int startVBO, int offsetVBO)
		{
			glEnableVertexAttribArray(location);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(location, locationSize, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void*)(startVBO * 8 * sizeof(float) + BUFFER_OFFSET(offsetVBO * sizeof(GLfloat))));
		}

		void linkCurrentBuffertoShader(GLuint VAOindex, int startVBO, int startIBO)
		{
			glBindVertexArray(VAOindex);

			bindVertexAttribute(loc1, 3, startVBO, 0);
			bindVertexAttribute(loc2, 3, startVBO, 3);
			bindVertexAttribute(loc3, 3, startVBO, 6);

			//IBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		}

		void linkCurrentBuffertoShaderCookTorrence(GLuint VAOindex, int startVBO, int startIBO)
		{
			glBindVertexArray(VAOindex);

			bindVertexAttribute(loc4, 3, startVBO, 0);
			bindVertexAttribute(loc5, 3, startVBO, 3);
			bindVertexAttribute(loc6, 3, startVBO, 6);

			//IBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		}
		
		void updateUniformVariablesPhong(glm::mat4 model)
		{
			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, &model[0][0]);

			//mat4 normalsTransform = transpose(inverse(model));
			//glUniformMatrix4fv(normals_location, 1, GL_FALSE, normalsTransform.m);
			//glUniformMatrix4fv(worldNormal, 1, GL_FALSE, normalsTransform.m);
		}

		void updateUniformVariablesPhong(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
		{
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
			updateUniformVariablesPhong(model);
		}

		void updateUniformVariablesBlinnPhong(glm::mat4 model)
		{
			glUniformMatrix4fv(model_mat_location2, 1, GL_FALSE, &model[0][0]);
		}

		void updateUniformVariablesBlinnPhong(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
		{
			glUniformMatrix4fv(proj_mat_location2, 1, GL_FALSE, &persp_proj[0][0]);
			glUniformMatrix4fv(view_mat_location2, 1, GL_FALSE, &view[0][0]);
			updateUniformVariablesBlinnPhong(model);
		}

		void updateUniformVariablesGooch(glm::mat4 model)
		{
			glUniformMatrix4fv(model_mat_location3, 1, GL_FALSE, &model[0][0]);
		}

		void updateUniformVariablesGooch(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
		{
			glUniformMatrix4fv(proj_mat_location3, 1, GL_FALSE, &persp_proj[0][0]);
			glUniformMatrix4fv(view_mat_location3, 1, GL_FALSE, &view[0][0]);
			updateUniformVariablesGooch(model);
		}

		void updateUniformVariablesCookTorrence(glm::mat4 model)
		{
			glUniformMatrix4fv(model_mat_location4, 1, GL_FALSE, &model[0][0]);
		}

		void updateUniformVariablesCookTorrence(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
		{
			glUniformMatrix4fv(proj_mat_location4, 1, GL_FALSE, &persp_proj[0][0]);
			glUniformMatrix4fv(view_mat_location4, 1, GL_FALSE, &view[0][0]);
			updateUniformVariablesCookTorrence(model);
		}

		void deleteVertexArrays()
		{
			//glDeleteVertexArrays(1, &footballbVAO);
			//glDeleteVertexArrays(1, &footballwVAO);	
		}

		void deleteBuffers()
		{
			glDeleteBuffers(1, &VBO);	
			glDeleteBuffers(1, &IBO);
		}

		void deletePrograms()
		{
			glDeleteProgram(PhongProgramID);
			glDeleteProgram(BlinnPhongID);
			glDeleteProgram(GoochID);
		}
	};
}