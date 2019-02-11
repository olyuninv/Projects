#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 reflectedVector; 
in vec3 refractedVector; 
in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;

in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

uniform samplerCube skybox;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{    
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0f, 1.0f, 1.0f);  // TODO: light color

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f); // TODO: light color
    
    // specular
    float specularStrength = 0.8;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 128);
    vec3 specular = 0.5 * spec *  vec3(1.0f, 1.0f, 1.0f);  // TODO: light color
        
    vec3 result = (ambient + diffuse + specular) * texture(diffuseTexture, TexCoord).rgb; 

    //FragColor = vec4 (result, 1.0);

    vec4 reflectedColour = texture(skybox, reflectedVector);
    vec4 refractedColour = texture(skybox, refractedVector);  
   
    // Works    
    float refractiveFactor = dot (viewDir, norm); 
    refractiveFactor = pow(refractiveFactor, 3);

    refractiveFactor = clamp (refractiveFactor, 0, 1);
    vec4 environmentColour = mix( reflectedColour, refractedColour, refractiveFactor); 

    //FragColor = mix(FragColor, environmentColour, 0.2f);  
    //FragColor = vec4(refractiveFactor, 0.0, 0.0, 0.0);

    //FragColor = texture(diffuseTexture, TexCoord);
    //FragColor = vec3(TexCoord, 0.0);

    // Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS) for better quality
	vec3 TextureNormal_tangentspace = normalize(texture(normalTexture, vec2(TexCoord.x,-TexCoord.y) ).rgb*2.0 - 1.0);
    FragColor = vec4(TextureNormal_tangentspace, 1.0);
    
    // Distance to the light
	float distance = length( viewPos - FragPos );

    // Normal of the computed fragment, in camera space
	vec3 n = TextureNormal_tangentspace;

    // Direction of the light (from the fragment to the light)
	vec3 l = normalize(LightDirection_tangentspace);
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );

    // Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_tangentspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );

    //FragColor = vec4 ( ambient + 
    //texture(diffuseTexture, TexCoord).rgb * cosTheta / (distance*distance) + 
    //0.5 * pow(cosAlpha, 5)/ (distance*distance), 1.0);
}