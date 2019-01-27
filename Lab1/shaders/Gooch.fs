#version 330 core

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos;
uniform vec3 warmColor;
uniform vec3 coolColor;

out vec4 FragColor;

void main()
{ 	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diffuseIntensity = dot(norm, lightDir);
    float weight = 0.5 * (1.0 + diffuseIntensity); 
        
    vec3 colorOut  = mix(coolColor, warmColor, weight);
    FragColor = vec4(colorOut, 1);
} 