#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

float iorRatioR = 1.51124;
float iorRatioG = 1.51534;
float iorRatioB = 1.51690;

out vec4 clipSpace;
out vec3 Normal;
out vec3 FragPos;
out vec3 toCameraVector;

out vec3 reflectedVector; 
//out vec3 refractedVector; 
out vec3 refractVecR;
out vec3 refractVecG;
out vec3 refractVecB;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0f);
	clipSpace = projection * view *  worldPosition;	
	gl_Position = clipSpace;
	  
	toCameraVector = viewPos -   worldPosition.xyz;

	FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;
    	
	vec3 unitNormal = normalize (Normal);
	
    vec3 viewDirection = normalize(worldPosition.xyz - viewPos);
            
    reflectedVector = reflect (viewDirection, unitNormal);
	//refractedVector = refract (viewDirection, unitNormal, 1.0/ 1.33);  // TODO: eta?
	refractVecR = refract(viewDirection, unitNormal, iorRatioR);
    refractVecG = refract(viewDirection, unitNormal, iorRatioG);
	refractVecB = refract(viewDirection, unitNormal, iorRatioB);
}