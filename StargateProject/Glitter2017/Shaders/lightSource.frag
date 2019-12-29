#version 330 core
out vec4 FragColor;

struct Light {
	//light attenuation for point lights  and spotlights using quadratic reduction
	int attenuationBool;
	float constant; 
    float linear;
    float quadratic;
};
uniform vec3 viewPos;
uniform vec3 color;
in vec3 FragPos;
uniform Light thisLight; 

void main()
{
	//attenuation
	float attenuation = 1; //default value
	if(thisLight.attenuationBool == 1){
		float distance = length(viewPos - FragPos);
		attenuation = 1.0 / (thisLight.constant + thisLight.linear * distance + thisLight.quadratic * (distance * distance));
	}
    FragColor = vec4(color * attenuation, 1.0f);
}