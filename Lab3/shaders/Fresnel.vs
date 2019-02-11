#version 330 core
in vec3 position;
in vec3 normal;
in vec2 texture;
in vec3 tangent;
in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 modelView3x3;
uniform vec3 viewPos;
uniform vec3 lightPos;

out vec2 TexCoord;   // UV
out vec3 Normal;  // ? 
out vec3 FragPos; //  Position_worldspace;

out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

out vec3 reflectedVector; 
out vec3 refractedVector; 

out vec3 LightDirection_tangentspace;
out vec3 EyeDirection_tangentspace;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0f);	
	gl_Position = projection * view *  worldPosition;
	
	FragPos = worldPosition.xyz;
   
	Normal = mat3(transpose(inverse(model))) * normal;
    	
	vec3 unitNormal = normalize (Normal);
	
    vec3 viewDirection = normalize(worldPosition.xyz - viewPos);
            
    reflectedVector = reflect (viewDirection, unitNormal);
	refractedVector = refract (viewDirection, unitNormal, 1.0/ 1.52);  // TODO: eta?

	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( view * worldPosition).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( view * vec4(lightPos,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

	TexCoord = texture;
	
	// model to camera = ModelView
	vec3 vertexTangent_cameraspace = modelView3x3 * tangent;
	vec3 vertexBitangent_cameraspace = modelView3x3 * bitangent;
	vec3 vertexNormal_cameraspace = modelView3x3 * normal;

	mat3 TBN = transpose(mat3(
		vertexTangent_cameraspace,
		vertexBitangent_cameraspace,
		vertexNormal_cameraspace	
	)); 

	LightDirection_tangentspace = TBN * LightDirection_cameraspace;
	EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;

}