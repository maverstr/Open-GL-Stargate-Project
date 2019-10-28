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
	int spotlight;
	vec3 direction;
    float innerCutOff;
	float outerCutOff;

	//light attenuation for point lights  and spotlights using quadratic reduction
	int attenuationBool;
	float constant; 
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 5 //for optimization purposes, I only render the 5 closest light sources

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 objectColor;
uniform vec3 viewPos;

uniform Material material;
uniform Light light[NR_POINT_LIGHTS]; 
uniform int lightCounter;

vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir);
vec3 calcEmission(void);

void main()
{//Using a function to calculate the lighting for each light source
    // vectors normalization
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result = vec3(0.0f, 0.0f, 0.0f); //default
    
	for (int i = 0; i < min(NR_POINT_LIGHTS, lightCounter); i++){
		result += calcFragFromALightSource(light[i], norm, FragPos, viewDir);    
    }
	result += calcEmission();
    FragColor = vec4(result, 1.0);
}


vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir){
//attenuation
	float attenuation = 1; //default value
	if(light.attenuationBool == 1){
		float distance = length(light.position.xyz - FragPos);
		attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	}
	//////////////////////////////PHONGS SHADING////////////////////////////////

	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// diffuse 
	vec3 lightDir = vec3(0.0f,0.0f,0.0f); //default value
	
	if(light.position.w == 1.0){ //if light is a point light
		lightDir = normalize(light.position.xyz - FragPos);
	}
	else if(light.position.w == 0.0){ //if light is directional (no impact from translations)
		lightDir = normalize(-light.position.xyz);
	}
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    

	// specular
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        


	//case of a spotlight
	if(light.spotlight == 1){ 
		float theta = dot(lightDir, normalize(-light.direction)); //cosine of angle between fragment and spot direction
		float intensity = smoothstep(light.outerCutOff, light.innerCutOff, theta);//progressive dark when out of inner cutoff angle cosine
		diffuse *= intensity;
		specular *= intensity;
		ambient *= intensity; //remove intensity for ambient so that inside is always lighter than outside spotlight cone
		}

	////////////////////////////RESULT////////////////////////////////////
	vec3 result = (ambient + diffuse + specular) * attenuation;
	return result;
}

vec3 calcEmission(void){
	// emission
	vec3 emission = texture(material.emission, TexCoords).rgb;
	return emission;
}