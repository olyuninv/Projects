#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 reflectedVector; 
in vec3 refractedVector; 
in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;

uniform sampler2D ourTexture;

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
        
    vec3 result = (ambient + diffuse + specular) * objectColor; 

    FragColor = vec4(result, 1.0);

    vec4 reflectedColour = texture(skybox, reflectedVector);
    vec4 refractedColour = texture(skybox, refractedVector);  
   
    // Schlick - does not work
    //float R0 = pow(((1 - 1.52)/ (1 + 1.52)), 2);        
    //float R0 = 0.31;
    //float refractiveFactor = 1 - dot (halfwayDir, norm);  // (viewDir, norm);  //norm, halfwayDir, 
    //refractiveFactor = pow(refractiveFactor, 5);
    //refractiveFactor = R0 + ((1 - R0) * refractiveFactor);

    // Works    
    float refractiveFactor = dot (viewDir, norm); 
    refractiveFactor = pow(refractiveFactor, 3);

    refractiveFactor = clamp (refractiveFactor, 0, 1);
    vec4 environmentColour = mix( reflectedColour, refractedColour, refractiveFactor); 
        
    //FragColor = mix(FragColor, environmentColour, 0.7f);  
    //FragColor = vec4(refractiveFactor, 0.0, 0.0, 0.0);

    FragColor = texture(ourTexture, TexCoord);
    //FragColor = vec3(TexCoord, 0.0);
}