#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 reflectedVector; 
//in vec3 refractedVector; 
in vec3 refractVecR;
in vec3 refractVecG;
in vec3 refractVecB;
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

    FragColor = vec4(result, 1.0);   //NOT USED LATER

    vec4 reflectedColour = texture(skybox, refractVecR);
    //vec4 refractedColour = texture(skybox, refractedVector);  

    vec4 color = vec4(0.0);
    color.r = texture(skybox, refractVecR).r;  // ADDED
    color.g = texture(skybox, refractVecG).g;  // ADDED
    color.b = texture(skybox, refractVecB).b;  // ADDED
    color.a = 1.0;


    vec3 viewVector = normalize(toCameraVector);
    float refractiveFactor = dot (viewVector, vec3 (1.0, 0.0, 1.0));
    refractiveFactor = pow(refractiveFactor, 3);

    vec4 environmentColour = mix(reflectedColour, color, refractiveFactor); //0.5f);  // ADDED
    //FragColor = environmentColour;
    //FragColor = mix (environmentColour, vec4(0.0, 0.3, 0.5, 1.0), 0.2);
    FragColor = mix(FragColor, environmentColour, 0.4f);  //refractedColour, 1.0); 
}