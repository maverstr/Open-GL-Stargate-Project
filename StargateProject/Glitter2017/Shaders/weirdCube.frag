#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
	float shininess;
	vec3 ambient; //material properties
	vec3 diffuse;
	vec3 specular;
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
    vec3 FragPos;
    vec2 TexCoords;
	mat3 TBN;
	vec3 Normal;
} fs_in;

uniform sampler2D normalMap;
uniform vec3 viewPos;
uniform Material material;
uniform Light light[NR_POINT_LIGHTS]; 
uniform int lightCounter;
uniform int normalMapping;

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

vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir, vec3 viewPos, bool sunlight);
float shadowCalculation(vec3 fragPos, vec3 lightPos, vec3 viewPos);


void main()
{//Using a function to calculate the lighting for each light source
    // vectors normalization
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 ViewDirEnvMapping = normalize(fs_in.FragPos - viewPos);
	vec3 result = vec3(0.0f, 0.0f, 0.0f); //default
    
	if(normalMapping == 1){
    norm = texture(normalMap, fs_in.TexCoords).rgb;
    // transform to range [-1,1] (from normal map i.e. RGB = [0,1])
    norm = normalize(norm * 2.0 - 1.0);  
	norm = normalize(fs_in.TBN * norm); 
	}

	for (int i = 0; i < min(NR_POINT_LIGHTS, lightCounter); i++){
		result += calcFragFromALightSource(light[i], norm, fs_in.FragPos, viewDir, viewPos, i == 3);    
    }

    FragColor = vec4(result, 1.0);
}


vec3 calcFragFromALightSource(Light light, vec3 norm, vec3 FragPos, vec3 viewDir, vec3 viewPos, bool sunlight){
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
	float shadow = 0.0f;
	if(sunlight){ //only for the sunlight
		shadow = shadowCalculation(fs_in.FragPos, vec3(light.position), viewPos); //equals 0 when frag not in shadow and 1 when in shadows, with soft shadows from PCF algo
	}
	else{
		shadow = 0.0f;
	}

	////////////////////////////RESULT////////////////////////////////////
	vec3 result = (ambient*0.1f + (1.0f - shadow) * (diffuse *0.8f + specular*1.2f)) * attenuation;
	return result;
}

float shadowCalculation(vec3 fragPos, vec3 lightPos, vec3 viewPos)
{    
	vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 4.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 10.0;
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