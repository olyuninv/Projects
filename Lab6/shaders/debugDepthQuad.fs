#version 330 core
out vec4 FragColor;

uniform float near; 
uniform float far; 
  
in vec2 TexCoords;

uniform sampler2D depthMap;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{             
    //float depth = LinearizeDepth(texture(depthMap, TexCoords).r) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
    FragColor = texture(depthMap, TexCoords);
} 