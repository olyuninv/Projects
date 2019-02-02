#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

float eta = 0.8;
varying vec3 R; 

void main()
{
    // gl_Position = projection * view *  model * vec4(position, 1.0f);
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;

	vec4 V = gl_ModelViewMatrix* gl_Vertex;
	vec4 E = gl_ProjectionMatrixInverse * vec4 (0, 0, -1, 0);
	vec3 I = normalize (V.xyz - E.xyz);
	vec3 N = normalize (gl_Normal );

	R = refract (I, N, eta);
	gl_Position = ftransform();
}