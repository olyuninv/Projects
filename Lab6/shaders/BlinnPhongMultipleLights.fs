#version 330 core

#extension GL_NV_shadow_samplers_cube : enable

#define NR_POINT_LIGHTS 2  

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

in vec3 FragPos;  
in vec2 TexCoord;


uniform vec3 viewPos;  // camera position in World space
uniform vec3 objectColor;

// for reflectance/ refraction
in vec3 Normal;  
in vec3 reflectedVector; 
in vec3 refractedVector; 

//uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

// for texture
in vec3 EyeDirection_cameraspace;
in vec3 EyeDirection_tangentspace;
in vec3 LightDirection_tangentspace[NR_POINT_LIGHTS];
in vec4 FragPosLightSpace[NR_POINT_LIGHTS];

//uniform samplerCube skybox;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform sampler2D shadowMap[NR_POINT_LIGHTS];

uniform bool useSolidColor;
uniform bool useNormalMap;
uniform bool useSpecularMap;

out vec4 FragColor;   // Final color

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // combine results
    vec3 ambient ;
    vec3 diffuse;
    vec3 specular ;
    if (useSolidColor)
    {
        ambient  = light.ambient  * objectColor;
        diffuse  = light.diffuse  * diff * objectColor;
        specular = light.specular * spec * objectColor;
     }
    else
    {
         ambient  = light.ambient  * vec3(texture(diffuseTexture, TexCoord));
         diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, TexCoord));
         specular = light.specular * spec * vec3(texture(specularTexture, TexCoord));
    }
     
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}  

vec3 CalcPointLight(PointLight light, 
            vec3 lightDir_tangent, vec3 normal,             
            vec3 fragPos, vec3 viewDir, 
            float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length( light.position - fragPos );
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // attenuation

    float attenuation = 1.0 / (light.constant + light.linear * distance + 
            light.quadratic * (distance * distance));    

    // combine results
    vec3 ambient ;
    vec3 diffuse;
    vec3 specular ;

    if (useSolidColor)
    {
        ambient  = light.ambient  * objectColor;
        diffuse  = light.diffuse  * diff * objectColor;
        specular = light.specular * spec * objectColor;
    }
    else
    {
        ambient  = light.ambient  * vec3(texture(diffuseTexture, TexCoord));
          
        if (useNormalMap)
        {
                vec3 TextureNormal_tangentspace = normalize(texture(normalTexture, vec2(TexCoord.x, -TexCoord.y) ).rgb*2.0 - 1.0);            

                vec3 n = TextureNormal_tangentspace;   // Normal of the computed fragment, in camera space    
                vec3 l = normalize(lightDir_tangent);  // Direction of the light (from the fragment to the light)   

                float cosTheta = clamp( dot( n,l ), 0,1 );  

                vec3 E = normalize(EyeDirection_tangentspace);   // from fragment towards the camera
                vec3 R = reflect(-l,n);                   // Direction in which the triangle reflects the light  

                float cosAlpha = clamp( dot( E,R ), 0,1 );

                // diffuseComponent          
                const float lightPower = 10.0f;
                diffuse =  lightPower * (vec3(texture(diffuseTexture, TexCoord)) * cosTheta) / (distance * distance);               

                // specularComponent        
                if (useSpecularMap)
                {   
                  // Tangent space
                    specular = light.specular * vec3(texture(specularTexture, TexCoord))  * pow(cosAlpha, 5)/ (distance*distance);
                }
                else
                {   
                    specular = light.specular * objectColor;
                }
        }
        else
        {
                ambient  = light.ambient  * vec3(texture(diffuseTexture, TexCoord));
                diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, TexCoord));
                specular = light.specular * spec * objectColor;
        }
    }
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadow_m)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 
    
    float closestDepth = texture(shadow_m, projCoords.xy).r;   

    float currentDepth = projCoords.z;  

    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;  

    return shadow;
}

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // phase 1: Directional lighting
    vec3 result;

    float shadow [NR_POINT_LIGHTS];
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        shadow[i] = ShadowCalculation(FragPosLightSpace[i], shadowMap[i]);       
    }

    result += CalcDirLight(dirLight, norm, viewDir, shadow[0]);
    
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], 
                LightDirection_tangentspace[i],
                norm, FragPos, viewDir, 
                shadow[i]); 
        
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
  
    // Depth buffer
    //FragColor = vec4(vec3(result.z), 1.0);
}