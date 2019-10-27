#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
	sampler2D emission;
    float     shininess;
};

struct Light {
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
	//case of spotlight
	vec3  direction;
    float cutOff;

	//light attenuation for point lights using quadratic reduction
	float attenuationBool;
	float constant; 
    float linear;
    float quadratic;
};

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 objectColor;
uniform vec3 viewPos;

uniform Material material;
uniform Light light; 



void main()
{
	//attenuation
	float attenuation = 1; //default value
	
	if(light.attenuationBool == 1.0f){
		float distance    = length(light.position.xyz - FragPos);
		attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	}
	//////////////////////////////PHONGS SHADING////////////////////////////////

    // ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));


    // diffuse 
    vec3 norm = normalize(Normal);
	vec3 lightDir = vec3(0.0f,0.0f,0.0f); //default value
	if(light.position.w == 1.0){ //if light is a point light
	    vec3 lightDir = normalize(light.position.xyz - FragPos);
	}
	else if(light.position.w == 0.0){ //if light is directional (no impact from translations)
		vec3 lightDir = normalize(-light.position.xyz);
	}
    float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        

	// emission
    vec3 emission = texture(material.emission, TexCoords).rgb;

	////////////////////////////RESULT////////////////////////////////////
    vec3 result = (ambient + diffuse + specular) * attenuation + emission;
	FragColor = vec4(result, 1.0); 
} 