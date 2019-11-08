#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1; //mat properties as texture maps
    sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
	sampler2D texture_emisson1;
	sampler2D texure_reflectionMap;
	int reflection;
	float shininess;
	vec3 ambient; //material properties
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float mixRatio; //use of texture combiner to render them all
	float refractionRatio;
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

#define NR_POINT_LIGHTS 10 //for optimization purposes, I only render the 10 closest light sources

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec3 NormalEnvMapping;
in vec3 Position;

uniform vec3 objectColor;
uniform vec3 viewPos;

uniform samplerCube skybox; //for refraction

uniform Material material;
uniform Light light[NR_POINT_LIGHTS]; 
uniform int lightCounter;

vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir);
vec3 calcEmission(void);
vec3 calcRefraction(vec3 normal, vec3 viewDir, float refractionRatio);
vec3 calcReflection(vec3 normal, vec3 viewDir);

void main()
{//Using a function to calculate the lighting for each light source
    // vectors normalization
    vec3 norm = normalize(Normal);
	vec3 normalEnvMapping = normalize(NormalEnvMapping);
    vec3 viewDir = normalize(viewPos - FragPos);
	vec3 ViewDirEnvMapping = normalize(Position - viewPos);
	vec3 result = vec3(0.0f, 0.0f, 0.0f); //default
    
	for (int i = 0; i < min(NR_POINT_LIGHTS, lightCounter); i++){
		result += calcFragFromALightSource(light[i], norm, FragPos, viewDir);    
    }
	result += calcEmission();

	//environment mapping
	result += calcReflection(normalEnvMapping, ViewDirEnvMapping) *3.0f;
	//result += calcRefraction(normalEnvMapping, ViewDirEnvMapping, material.refractionRatio);

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
	vec3 ambient = light.ambient * mix(vec3(texture(material.texture_diffuse1, TexCoords)), material.diffuse, material.mixRatio);

	// diffuse 
	vec3 lightDir = vec3(0.0f,0.0f,0.0f); //default value
	
	if(light.position.w == 1.0){ //if light is a point light
		lightDir = normalize(light.position.xyz - FragPos);
	}
	else if(light.position.w == 0.0){ //if light is directional (no impact from translations)
		lightDir = normalize(-light.position.xyz);
	}
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * mix(vec3(texture(material.texture_diffuse1, TexCoords)),material.diffuse, material.mixRatio);
    

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir); //used for Blinn-Phong's shading algorithm to overcome issues with high shininess
	float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess); 
	vec3 specular = light.specular * spec * mix(vec3(texture(material.texture_specular1, TexCoords)), material.specular, material.mixRatio);
        

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
	vec3 emission = mix(texture(material.texture_emisson1, TexCoords).rgb, material.emission, material.mixRatio);
	return emission;
}

vec3 calcReflection(vec3 normal, vec3 viewDir){
	//refraction
	vec3 R = reflect(viewDir, normal); //using OpenGL built-in function, algebra is similar to specular light
	vec3 reflection = vec3(texture(skybox, R).rgb);
	if(material.reflection == 1)
		reflection = reflection * texture(material.texure_reflectionMap, TexCoords).x; //map for reflection values
	else
		reflection = vec3(0,0,0);
	return reflection;
}

vec3 calcRefraction(vec3 normal, vec3 viewDir, float refractionRatio){
	//refraction
	vec3 R = refract(viewDir, normal, refractionRatio); //using built-in fct
	vec3 refraction = vec3(texture(skybox, R).rgb);
	return refraction;
}
