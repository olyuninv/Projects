#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

float iorRatioR = 1.51;
float iorRatioG = 1.52;
float iorRatioB = 1.53;

out vec4 clipSpace;
out vec3 Normal;
out vec3 FragPos;

out vec3 reflectedVector; 
out vec3 refractVecR;
out vec3 refractVecG;
out vec3 refractVecB;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0f);
	clipSpace = projection * view *  worldPosition;	
	gl_Position = clipSpace;

	FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;
    	
	vec3 unitNormal = normalize (Normal);
	
    vec3 viewDirection = normalize(worldPosition.xyz - viewPos);
            
    reflectedVector = reflect (viewDirection, unitNormal);
	refractVecR = refract(viewDirection, unitNormal, 1.0 / (iorRatioR - 0.3));
    refractVecG = refract(viewDirection, unitNormal, 1.0 / (iorRatioG - 0.2));
	refractVecB = refract(viewDirection, unitNormal, 1.0 / (iorRatioB - 0.1));
}