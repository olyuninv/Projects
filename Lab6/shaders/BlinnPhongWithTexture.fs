#version 330 core

#extension GL_NV_shadow_samplers_cube : enable

in vec3 FragPos;  
in vec2 TexCoord;

uniform vec3 viewPos;  // camera position in World space
uniform vec3 lightPos; // light position in World space
uniform vec3 lightColor;
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

uniform bool useSolidColor;
uniform bool useNormalMap;
uniform bool useSpecularMap;

out vec4 FragColor;   // Final color

void main()
{    
      // ambientComponent

      float ambientStrength = 0.1;
      vec3 ambientComponent = ambientStrength * lightColor;     // TODO: move light color into variable
      ambientComponent = ambientComponent * objectColor;  //- ignore object color for now

      vec3 norm = normalize(Normal);
      vec3 lightDir = normalize(lightPos - FragPos);      

      vec3 diffuseComponent;
      vec3 specularComponent;
            
      float specularStrength = 0.5;
      vec3 MaterialDiffuseColor =  texture(diffuseTexture, vec2(TexCoord.x,  -TexCoord.y)).rgb;    

      vec3 viewDir = normalize(viewPos - FragPos);
      
      if (useSolidColor)
      {
            // diffuse - World space
            float diff = max(dot(norm, lightDir), 0.0);
            diffuseComponent = diff * lightColor; 
            diffuseComponent = diffuseComponent * objectColor;
           
            // specular - World space          
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);  // Shininess
            specularComponent = specularStrength * spec *  lightColor;  
            specularComponent = specularComponent * objectColor;  
      }
      else
      {
          if (useNormalMap)
          {
              vec3 TextureNormal_tangentspace = normalize(texture(normalTexture, vec2(TexCoord.x, -TexCoord.y) ).rgb*2.0 - 1.0);            

	          float distance = length( lightPos - FragPos );     // Distance to the light    

              vec3 n = TextureNormal_tangentspace;   // Normal of the computed fragment, in camera space    
	          vec3 l = normalize(LightDirection_tangentspace);  // Direction of the light (from the fragment to the light)   

	          float cosTheta = clamp( dot( n,l ), 0,1 );  

	          vec3 E = normalize(EyeDirection_tangentspace);   // from fragment towards the camera
	          vec3 R = reflect(-l,n);                   // Direction in which the triangle reflects the light  

              float cosAlpha = clamp( dot( E,R ), 0,1 );

              // diffuseComponent          
              const float lightPower = 10.0f;
	          diffuseComponent =  lightPower * (MaterialDiffuseColor * cosTheta) / (distance * distance);               

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
                  specularComponent = specularStrength * spec *  lightColor; 
                  specularComponent = specularComponent * objectColor;
              }
          }
          else
          {
               // diffuse - World space
               float diff = max(dot(norm, lightDir), 0.0);
               diffuseComponent = diff * lightColor; 
               diffuseComponent = diffuseComponent * MaterialDiffuseColor;
               
               // specular - World space          
               vec3 halfwayDir = normalize(lightDir + viewDir);
               float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);  // Shininess
               specularComponent = specularStrength * spec *  lightColor;  
               specularComponent = specularComponent * objectColor;  
          }
      
      }   

      FragColor = vec4 (ambientComponent + diffuseComponent + specularComponent, 1.0);      


      //// Reflectance/ Refraction -- comment out for now
      //// compute environment color
      //vec4 reflectedColour = texture(skybox, reflectedVector);
      //vec4 refractedColour = texture(skybox, refractedVector);    

      //// Fresnel    
      //float refractiveFactor = dot (viewDir, norm); 
      //refractiveFactor = pow(refractiveFactor, 3);
        
      //refractiveFactor = clamp (refractiveFactor, 0, 1);
      //vec4 environmentColour = mix( reflectedColour, refractedColour, refractiveFactor);
      //FragColor = mix(FragColor, environmentColour, 0.1f);  
}