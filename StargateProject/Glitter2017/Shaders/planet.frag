#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1; //mat properties as texture maps
	float shininess;
	vec3 ambient; //material properties
	vec3 diffuse;
	vec3 specular;

	int reflection;
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
in VS_OUT {
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
} fs_in;


uniform vec3 viewPos;
uniform Material material;
uniform Light light[NR_POINT_LIGHTS]; 
uniform int lightCounter;

//shadows
uniform float far_plane;
uniform samplerCube depthMap;
// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

uniform samplerCube skybox; //for refraction

vec3 calcRefraction(vec3 normal, vec3 viewDir, float refractionRatio);
vec3 calcReflection(vec3 normal, vec3 viewDir);
vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir, vec3 viewPos);
float shadowCalculation(vec3 fragPos, vec3 lightPos, vec3 viewPos);



void main()
{//Using a function to calculate the lighting for each light source
    // vectors normalization
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 ViewDirEnvMapping = normalize(fs_in.FragPos - viewPos);
	vec3 result = vec3(0.0f, 0.0f, 0.0f); //default
    
	for (int i = 0; i < min(NR_POINT_LIGHTS, lightCounter); i++){
		result += calcFragFromALightSource(light[i], norm, fs_in.FragPos, viewDir, viewPos);    
    }

	//environment mapping
	result += calcReflection(norm, ViewDirEnvMapping) *2.5f; //used only if triggered for demo purpose
	if(material.refractionRatio != 0.0)
		result = calcRefraction(norm, ViewDirEnvMapping, material.refractionRatio);  //used only if triggered for demo purpose, only refraction
    FragColor = vec4(result, 1.0);
	//FragColor = vec4(test,1.0f); debug purpose
}


vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir, vec3 viewPos){
//attenuation
	float attenuation = 1; //default value
	if(light.attenuationBool == 1){
		float distance = length(light.position.xyz - FragPos);
		attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	}
	//////////////////////////////PHONGS SHADING////////////////////////////////

	// ambient
	vec3 ambient = vec3(texture(material.texture_diffuse1, fs_in.TexCoords));

	// diffuse 
	vec3 lightDir = vec3(0.0f,0.0f,0.0f); //default value
	
	if(light.position.w == 1.0f){ //if light is a point light
		lightDir = normalize(light.position.xyz - FragPos);
	}
	else if(light.position.w == 0.0f){ //if light is directional (no impact from translations)
		lightDir = normalize(-light.position.xyz);
	}
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir); //used for Blinn-Phong's shading algorithm to overcome issues with high shininess
	float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess); 
	vec3 specular = light.specular * spec;
        

	//case of a spotlight
	if(light.spotlight == 1){ 
		float theta = dot(lightDir, normalize(-light.direction)); //cosine of angle between fragment and spot direction
		float intensity = smoothstep(light.outerCutOff, light.innerCutOff, theta);//progressive dark when out of inner cutoff angle cosine
		diffuse *= intensity;
		specular *= intensity;
		ambient *= intensity; //remove intensity for ambient so that inside is always lighter than outside spotlight cone
		}

	//shadows
	float shadow = shadowCalculation(fs_in.FragPos, vec3(light.position), viewPos); //equals 0 when frag not in shadow and 1 when in shadows, with soft shadows from PCF algo
	////////////////////////////RESULT////////////////////////////////////
	vec3 result = (ambient*0.2f + (1.0f - shadow) * (diffuse *1.8 + specular*0.8)) * attenuation;
	return result;
}


vec3 calcReflection(vec3 normal, vec3 viewDir){
	//refraction
	vec3 reflection = vec3(0.0f);
	if(material.reflection == 1){
	vec3 R = reflect(viewDir, normal); //using OpenGL built-in function, algebra is similar to specular light
	reflection = vec3(texture(skybox, R).rgb);
	}
	return reflection;
}

vec3 calcRefraction(vec3 normal, vec3 viewDir, float refractionRatio){
	//refraction
	vec3 refraction = vec3(0.0f);
	if(material.refractionRatio != 0.0f){
	vec3 R = refract(viewDir, normal, refractionRatio); //using built-in fct
	refraction = vec3(texture(skybox, R).rgb);
	}
	return refraction;
}

float shadowCalculation(vec3 fragPos, vec3 lightPos, vec3 viewPos)
{    
	vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 4.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 3.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane; 
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    return shadow;
} 