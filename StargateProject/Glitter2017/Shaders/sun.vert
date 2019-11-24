 #version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec2 viewportPixelCoord;
out vec2 sunPosViewportPixelCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 sunPos;



void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f); 

	vec3 ndc = gl_Position.xyz / gl_Position.w; //perspective divide/normalize
	vec2 viewportCoord = ndc.xy * 0.5 + 0.5; //ndc is -1 to 1 in GL. scale for 0 to 1
	viewportPixelCoord = viewportCoord;

	vec4 sunPosition = projection * view * vec4(sunPos, 1.0f); //no need for model here since sunpos il already translated intrinsecally
	ndc = sunPosition.xyz / sunPosition.w; //perspective divide/normalize
	sunPosViewportPixelCoord = ndc.xy * 0.5 + 0.5; //ndc is -1 to 1 in GL. scale for 0 to 1; 
}