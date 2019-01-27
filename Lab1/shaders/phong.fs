#version 330 core

in vec3 Normal;  
in vec3 FragPos;  
  
uniform float ambientCoef;
uniform float diffuseCoef;
uniform float specularCoef;
uniform int shininess;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 FragColor;

void main()
{
    // ambient    
    vec3 ambient = ambientCoef * lightColor;
  	
    // diffuse     
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse =  diffuseCoef * diff * lightColor;
    
    // specular    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularCoef * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 