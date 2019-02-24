#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in vec3 FragPos;  
in vec2 TexCoord;

uniform vec3 viewPos;  // camera position in World space
uniform vec3 lightPos; // light position in World space
uniform vec3 objectColor;

// for reflectance/ refraction
in vec3 Normal;  
in vec3 reflectedVector; 
in vec3 refractedVector; 

// for texture
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

uniform samplerCube skybox;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform bool useNormalMap;
uniform bool useSpecularMap;

out vec4 FragColor;   // Final color

void main()
{    
      // ambientComponent
      float ambientStrength = 0.1;
      vec3 ambientComponent = ambientStrength * vec3(1.0f, 1.0f, 1.0f);     // TODO: move light color into variable
      ambientComponent = ambientComponent * objectColor;  //- ignore object color for now

      vec3 norm = normalize(Normal);
      vec3 lightDir = normalize(lightPos - FragPos);
      
      vec3 diffuseComponent;
      vec3 specularComponent;

      float specularStrength = 0.5;
      float specularPower = 2.0;
      vec3 MaterialDiffuseColor =  texture(diffuseTexture, vec2(TexCoord.x,  -TexCoord.y)).rgb;    

      if (useNormalMap)
      {
          // Tangent space:

          // Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS) for better quality      
          vec3 TextureNormal_tangentspace = normalize(texture(normalTexture, vec2(TexCoord.x, -TexCoord.y) ).rgb*2.0 - 1.0);
            
	      float distance = length( lightPos - FragPos );     // Distance to the light
    
          vec3 n = TextureNormal_tangentspace;   // Normal of the computed fragment, in camera space    
	      vec3 l = normalize(LightDirection_tangentspace);  // Direction of the light (from the fragment to the light)
    
	      // Cosine of the angle between the normal and the light direction, clamped above 0
	      //  - light is at the vertical of the triangle -> 1, light is perpendicular to the triangle -> 0, light is behind the triangle -> 0
	      float cosTheta = clamp( dot( n,l ), 0,1 );
    
	      vec3 E = normalize(EyeDirection_tangentspace);   // from fragment towards the camera
	      vec3 R = reflect(-l,n);                   // Direction in which the triangle reflects the light
	  
          // Cosine of the angle between the Eye vector and the Reflect vector, clamped to 0
	      //  - Looking into the reflection -> 1, Looking elsewhere -> < 1
	      float cosAlpha = clamp( dot( E,R ), 0,1 );

          // diffuseComponent          
          
	      diffuseComponent =  specularPower * MaterialDiffuseColor * cosTheta / (distance*distance);
               
          // specularComponent          
          if (useSpecularMap)
          {   
              // Tangent space
              vec3 MaterialSpecularColor = texture(specularTexture, vec2(TexCoord.x, -TexCoord.y)).rgb ;                  
              specularComponent = specularStrength * MaterialSpecularColor * pow(cosAlpha, 5)/ (distance*distance);
          }
          else
          {
	          // World space
              vec3 viewDir = normalize(viewPos - FragPos);
              vec3 halfwayDir = normalize(lightDir + viewDir);
              float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);  // Shininess
              specularComponent = specularStrength * spec *  vec3(1.0f, 1.0f, 1.0f);  // TODO: light color
              specularComponent = specularComponent * objectColor;
          }
      }
      else
      {
           // diffuse - World space
           float diff = max(dot(norm, lightDir), 0.0);
           diffuseComponent = diff * vec3(1.0f, 1.0f, 1.0f); // TODO: light color, diffuse coefficient
           diffuseComponent = specularPower * diffuseComponent * MaterialDiffuseColor;

           // specular - World space
           vec3 viewDir = normalize(viewPos - FragPos);
           vec3 halfwayDir = normalize(lightDir + viewDir);
           float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);  // Shininess
           specularComponent = specularStrength * spec *  vec3(1.0f, 1.0f, 1.0f);  // TODO: light color
           specularComponent = specularComponent * objectColor;
      }
            
      FragColor = vec4 (ambientComponent + diffuseComponent + specularComponent, 1.0);

      // Reflectance/ Refraction -- comment out for now
      // compute environment color
        //vec4 reflectedColour = texture(skybox, reflectedVector);
        //vec4 refractedColour = texture(skybox, refractedVector);  
   
        // Fresnel    
        //float refractiveFactor = dot (viewDir, norm); 
        //refractiveFactor = pow(refractiveFactor, 3);

        //refractiveFactor = clamp (refractiveFactor, 0, 1);
        //vec4 environmentColour = mix( reflectedColour, refractedColour, refractiveFactor); 
        //FragColor = mix(FragColor, environmentColour, 0.1f);  

}