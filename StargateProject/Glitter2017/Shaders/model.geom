#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//note: gl_in is a built-in variable
//note2: seems like geom shader is hyper fast as shapes are generated dynamically... might be a good optimization...
//		for simple repetitive models and just use a point in the VBO...


in VS_OUT {
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
	vec3 NormalInMVP;
} gs_in[];

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 NormalInMVP;
out vec3 test;

/*
//simple pass-through geom shader for testing...
void main() {    
    gl_Position = gl_in[0].gl_Position; 
	TexCoords = gs_in[0].TexCoords;
	FragPos = gs_in[0].FragPos;
	Normal = gs_in[0].Normal;
	NormalInMVP = gs_in[0].NormalInMVP;
    EmitVertex();
	gl_Position = gl_in[1].gl_Position; 
	TexCoords = gs_in[1].TexCoords;
	FragPos = gs_in[1].FragPos;
	Normal = gs_in[1].Normal;
	NormalInMVP = gs_in[1].NormalInMVP;
    EmitVertex();
	gl_Position = gl_in[2].gl_Position; 
	TexCoords = gs_in[2].TexCoords;
	FragPos = gs_in[2].FragPos;
	Normal = gs_in[2].Normal;
	NormalInMVP = gs_in[2].NormalInMVP;
    EmitVertex();
    EndPrimitive();
}
*/

uniform float explosionDistance;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 5.0;
    vec3 direction = normal * ((explosionDistance + 1.0) / 2.0) * magnitude; 
	return position + vec4(direction,0.0f);
}
/*
vec3 testFct(vec3 normal){
    float magnitude = 5.0;
    vec3 direction = normal * ((explosionDistance + 1.0) / 2.0) * magnitude; 
	return direction;
}
*/

void main() {
    gl_Position = explode(gl_in[0].gl_Position, gs_in[0].NormalInMVP);
	TexCoords = gs_in[0].TexCoords;
	FragPos = gs_in[0].FragPos;
	Normal = gs_in[0].Normal;
	NormalInMVP = gs_in[0].NormalInMVP;
	//test = testFct(gs_in[0].NormalInMVP);
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, gs_in[1].NormalInMVP);
	TexCoords = gs_in[1].TexCoords;
	FragPos = gs_in[1].FragPos;
	Normal = gs_in[1].Normal;
	NormalInMVP = gs_in[1].NormalInMVP;
	//test = testFct(gs_in[1].NormalInMVP);
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, gs_in[2].NormalInMVP);
	TexCoords = gs_in[2].TexCoords;
	FragPos = gs_in[2].FragPos;
	Normal = gs_in[2].Normal;
	NormalInMVP = gs_in[2].NormalInMVP;
	//test = testFct(gs_in[2].NormalInMVP);
    EmitVertex();
    EndPrimitive();
}