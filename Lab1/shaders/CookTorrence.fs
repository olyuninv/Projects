#version 330 core

#define PI 3.14159265

in vec3 Normal;  
in vec3 FragPos;  
  
uniform float ambientCoef;
uniform float diffuseCoef;
uniform float specularCoef;
//uniform int shininess;

uniform float metallic;
uniform float roughness;
//uniform float k = 0.2;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 FragColor;

void main()
{
     // ambient
    vec3 ambient = ambientCoef * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float diff = max(dot(norm, lightDir), 0.0);
    float Rs = 0.0;
    float NdotH = 0.0;
  	float NdotV = 0.0;
    float VdotH = 0.0;
    float F = 0.0;
    
    float k = 0.2;

    if (diff > 0) 
    {
        NdotH = max(dot(norm, halfwayDir), 0.0);
        NdotV = max(dot(norm, viewDir), 0.0);
        VdotH = max(dot(viewDir, halfwayDir), 0.0); //? - updated VdotH

        // Fresnel reflectance
        F = pow(1.0 - VdotH, 5.0);
        F *= (1.0 - metallic);
        F += metallic;  // wrong formulae

        // Microfacet distribution by Beckmann
        float m_squared = roughness * roughness;
        float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
        float D = r1 * exp(r2);

        // Geometric shadowing
        float two_NdotH = 2.0 * NdotH;
        float g1 = (two_NdotH * NdotV) / VdotH;
        float g2 = (two_NdotH * diff) / VdotH;
        float G = min(1.0, min(g1, g2));

        Rs = (F * D * G) / (PI * diff * NdotV);
    }
   
    // diffuse     
    vec3 diffuse = diffuseCoef * diff * lightColor;
    
    // specular    
    vec3 specular = lightColor * diff * (k + Rs * (1.0 - k));

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 