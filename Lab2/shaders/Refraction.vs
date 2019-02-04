#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

out vec3 Normal;
out vec3 FragPos;

out vec3 pass_normal;
//out vec3 reflectedVector; 
out vec3 refractedVector; 

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0f);
	
	gl_Position = projection * view *  worldPosition;
	
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;
	vec3 unitNormal = normalize (Normal);  //normal
    pass_normal = Normal; //normal;
	
    vec3 viewDirection = normalize(worldPosition.xyz - viewPos);
            
   // reflectedVector = reflect (viewDirection, unitNormal); 
	refractedVector = refract (viewDirection, unitNormal, 1.0/ 1.33); 
}