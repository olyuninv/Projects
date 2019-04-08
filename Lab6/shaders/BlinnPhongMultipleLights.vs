#version 330 core

#define NR_POINT_LIGHTS 2  

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 position;	// Model space
in vec3 normal;		// Model space
in vec2 texture;	// Model space
in vec3 tangent;	// Model space
in vec3 bitangent;	// Model space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;  
uniform mat3 modelView3x3;  // View * Model to bring everything to cameraspace

uniform vec3 viewPos;	// Position of the camera in world space
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform mat4 lightSpaceMatrix;

out vec2 TexCoord;   // UV
out vec3 FragPos; //  Position_worldspace;

// Need to refraction/ reflection
out vec3 Normal;  
out vec3 reflectedVector; 
out vec3 refractedVector; 

out vec3 EyeDirection_cameraspace;
out vec3 EyeDirection_tangentspace; 
out vec3 LightDirection_tangentspace[NR_POINT_LIGHTS];

out vec4 FragPosLightSpace; //  Position_worldspace;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0f);	
	gl_Position = projection * view *  worldPosition;
	
	TexCoord = texture;
	FragPos = worldPosition.xyz;
	   
	Normal = mat3(transpose(inverse(model))) * normal;
    	
	vec3 unitNormal = normalize (Normal);
	
    vec3 viewDirection = normalize(worldPosition.xyz - viewPos);
            
    reflectedVector = reflect (viewDirection, unitNormal);
	refractedVector = refract (viewDirection, unitNormal, 1.0/ 1.52);  // TODO: eta?

	// Used for Directional light shadow
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
		
	// Textures:
	vec3 vertexPosition_cameraspace = ( view * worldPosition).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 tangent_up = normalize(tangent - (normal * dot(normal, tangent)));

	//if (dot(cross(normal, tangent_up), bitangent) < 0.0f){
		//	tangent_up = tangent_up * -1.0f;
    //}
	
	// model to camera = ModelView
	vec3 vertexTangent_cameraspace = modelView3x3 * tangent_up;
	vec3 vertexBitangent_cameraspace = modelView3x3 * bitangent;
	vec3 vertexNormal_cameraspace = modelView3x3 * normal;

	mat3 TBN = transpose(mat3(
		vertexTangent_cameraspace,
		vertexBitangent_cameraspace,
		vertexNormal_cameraspace	
	)); 
	
	EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;
		
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		vec3 LightPosition_cameraspace = ( view * vec4(pointLights[i].position, 1)).xyz;
		vec3 LightDirection_cameraspace = normalize (LightPosition_cameraspace + EyeDirection_cameraspace);
	
		LightDirection_tangentspace[i] = TBN * LightDirection_cameraspace;
	}
}