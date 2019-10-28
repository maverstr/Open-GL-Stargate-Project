#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 objectColor;
uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light; 

void main()
{
    // ambient
	vec3 ambient  = light.ambient * material.ambient;
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse  = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
	vec3 specular = light.specular * (spec * material.specular);  	
        
    vec3 result = ambient + diffuse + specular;
	
	//FragColor = vec4(result, 1.0) * vec4(1.0f,0.1f,0.3f,1.0f);
	FragColor =  vec4(result, 1.0)  * texture(texture_diffuse1, TexCoords);
} 