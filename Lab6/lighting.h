#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lab6
{
	struct BaseLight
	{
		glm::vec3 color;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		BaseLight()
		{
			color = glm::vec3(0.0f, 0.0f, 0.0f);
			ambient = glm::vec3(0.0f, 0.0f, 0.0f);
			diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	};

	struct DirectionalLight: public BaseLight
	{		
		glm::vec3 direction;
		
		DirectionalLight()
		{
			direction = glm::vec3(0.0f, 0.0f, 0.0f);
		}		
	};

	struct PointLight : public BaseLight
	{
		glm::vec3 position;

		struct
		{
			float constant;
			float linear;
			float exp;
		} attenuation;

		PointLight()
		{
			position = glm::vec3(0.0f, 0.0f, 0.0f);
			attenuation.constant = 1.0f;
			attenuation.linear = 0.0f;
			attenuation.exp = 0.0f;
		}
	};
}