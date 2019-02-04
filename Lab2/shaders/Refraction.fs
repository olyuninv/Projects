#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

in  vec3 R;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{    
    FragColor = textureCube(skybox, R);
}