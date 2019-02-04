#include "CGObject.h"
namespace Lab2
{
	CGObject::CGObject()
	{
	}

	CGObject::~CGObject()
	{
	}

	void CGObject::Draw(opengl_utils glutils, GLuint programID)
	{
		int VBOindex = this->startVBO;
		int IBOindex = this->startIBO;
		for (int i = 0; i < this->Meshes.size(); i++) {
			
			if (programID == glutils.PhongProgramID)
			{
				glutils.linkCurrentBuffertoShader(this->VAOs[i], VBOindex, IBOindex);
			}
			else if (programID == glutils.ReflectionID)
			{
				glutils.linkCurrentBuffertoShaderReflectance(this->VAOs[i], VBOindex, IBOindex);
			}

			glDrawElements(GL_TRIANGLES, this->Meshes[i].Indices.size(), GL_UNSIGNED_INT, (void*)(IBOindex * sizeof(unsigned int)));
			VBOindex += Meshes[i].Vertices.size();
			IBOindex += Meshes[i].Indices.size();
		}
	}

	glm::mat4 CGObject::createTransform()
	{
		glm::mat4 localTransform = glm::mat4(1.0);
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.x + this->rotateAngles.x, glm::vec3(1, 0, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.y + this->rotateAngles.y, glm::vec3(0, 1, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.z + this->rotateAngles.z, glm::vec3(0, 0, 1));
		localTransform = glm::translate(localTransform, this->position);
		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;
		return parentTransform * localTransform;
	}
}