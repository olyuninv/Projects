#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

float eta = 0.8;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;
out vec3 R; 

void main()
{
    // gl_Position = projection * view *  model * vec4(position, 1.0f);
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;

	vec4 V = view * model * vec4(position, 1.0f);
	vec4 E = inverse (projection) * vec4 (0, 0, -1, 0);
	
	vec3 I = normalize (V.xyz - E.xyz);
	
	vec3 N = normalize (Normal);

	// Refraction formulae (from GLSL documentation)
	//k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
    //if (k < 0.0)
    //    R = genType(0.0);       // or genDType(0.0)
    //else
    //    R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;

	R = refract (I, N, eta);
	gl_Position = invariant;
}