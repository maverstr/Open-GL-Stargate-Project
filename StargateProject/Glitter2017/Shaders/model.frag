#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
  
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;

void main()
{
    // ambient
    vec3 ambient = lightColor * 0.2f;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff);
    
    // specular
	float specularStrength = 0.99f;
	int shininess = 32;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); //material.shininess
    vec3 specular = lightColor * (spec * specularStrength);   //material.specular
        
    vec3 result = ambient + diffuse + specular;
	
	//FragColor = vec4(result, 1.0) * vec4(1.0f,0.1f,0.3f,1.0f);
	FragColor =  vec4(result, 1.0)  * texture(texture_diffuse1, TexCoords);
} 