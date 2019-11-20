#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
	vec3 NormalInMVP;
} vs_out;


uniform mat4 transMat;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
    vs_out.NormalInMVP = normalize(vec3(projection * vec4(mat3(transpose(inverse(view * model))) * aNormal, 0.0)));
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}