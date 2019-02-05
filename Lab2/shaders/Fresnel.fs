#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 reflectedVector; 
in vec3 refractedVector; 
in vec3 Normal;  
in vec3 FragPos;  
in vec3 toCameraVector;

uniform samplerCube skybox;
uniform vec3 lightPos;
uniform vec3 objectColor;

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
    
    vec3 result = (ambient + diffuse) * objectColor; 

    FragColor = vec4(result, 1.0);

    vec4 reflectedColour = texture(skybox, reflectedVector);
    vec4 refractedColour = texture(skybox, refractedVector);  // ADDED

    vec3 viewVector = normalize(toCameraVector);
    float refractiveFactor = dot (viewVector, vec3 (1.0, 1.0, 1.0));
    refractiveFactor = pow(refractiveFactor, 3.0);

    vec4 environmentColour = mix(reflectedColour, refractedColour, refractiveFactor); //0.5f);  // ADDED

    //FragColor = mix (environmentColour, vec4(0.0, 0.3, 0.5, 1.0), 0.2);
    FragColor = mix(FragColor, environmentColour, 1.0f);  //refractedColour, 1.0); 
}