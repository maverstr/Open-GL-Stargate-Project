#version 330 core
out vec4 FragColor;
struct Material {
	float shininess;
	vec3 ambient; //material properties
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float mixRatio; //use of texture combiner to render them all
};

uniform Material material;

void main()
{
	FragColor = vec4(material.diffuse,1.0f);
}