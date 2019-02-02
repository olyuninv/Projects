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

namespace Lab2
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

		// Shaders
		GLuint PhongProgramID;
		GLuint CubeMapID;

		// Buffers
		GLuint VBO;
		GLuint IBO;

		// Uniform locations
		GLint model_mat_location;
		GLint view_mat_location;
		GLint proj_mat_location;
		GLint objectColorLoc;
		GLint lightColorLoc;
		GLint lightPosLoc;
		GLint viewPosLoc;

		// Uniform - cubemap
		GLint view_mat_location2;
		GLint proj_mat_location2;

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
			PhongProgramID = LoadShaders("../Lab2/shaders/phong.vs", "../Lab2/shaders/phong.fs");
			CubeMapID = LoadShaders("../Lab2/shaders/cubeMap.vs", "../Lab2/shaders/cubeMap.fs");
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
			//Declare your uniform variables that will be used in your shader
			model_mat_location = glGetUniformLocation(PhongProgramID, "model");
			view_mat_location = glGetUniformLocation(PhongProgramID, "view");
			proj_mat_location = glGetUniformLocation(PhongProgramID, "projection");

			objectColorLoc = glGetUniformLocation(PhongProgramID, "objectColor");
			lightColorLoc = glGetUniformLocation(PhongProgramID, "lightColor");
			lightPosLoc = glGetUniformLocation(PhongProgramID, "lightPos");
			viewPosLoc = glGetUniformLocation(PhongProgramID, "viewPos");

			view_mat_location2 = glGetUniformLocation(CubeMapID, "view");
			proj_mat_location2 = glGetUniformLocation(CubeMapID, "projection");
		}

		void getAttributeLocations()
		{
			loc1 = glGetAttribLocation(PhongProgramID, "position");
			loc2 = glGetAttribLocation(PhongProgramID, "normal");
			loc3 = glGetAttribLocation(PhongProgramID, "texture");

			loc4 = glGetAttribLocation(CubeMapID, "aPos");
		}

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
		
		void updateUniformVariables(glm::mat4 model)
		{
			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, &model[0][0]);

			//mat4 normalsTransform = transpose(inverse(model));
			//glUniformMatrix4fv(normals_location, 1, GL_FALSE, normalsTransform.m);
			//glUniformMatrix4fv(worldNormal, 1, GL_FALSE, normalsTransform.m);
		}

		void updateUniformVariables(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
		{
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
			updateUniformVariables(model);
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
//			glDeleteProgram(lightingID);
		}
	};
}