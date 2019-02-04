#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 pass_normal;
in vec3 reflectedVector; 
in vec3 refractedVector; 
in vec3 Normal;  
in vec3 FragPos;  

uniform samplerCube skybox;
uniform vec3 lightPos;

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
    
    vec3 result = (ambient + diffuse) * vec3(1.0f, 1.0f, 1.0f); // TODO: object color

    FragColor = vec4(result, 1.0);

    vec4 reflectedColour = texture(skybox, reflectedVector);
    vec4 refractedColour = texture(skybox, refractedVector);  // ADDED

    vec4 environmentColour = mix(reflectedColour, refractedColour, 0.5f);  // ADDED

    FragColor = mix(FragColor, environmentColour, 1.0f);  //refractedColour, 1.0); 
}