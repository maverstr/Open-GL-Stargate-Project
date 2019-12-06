 #version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec2 viewportPixelCoord;
out vec2 PosViewportPixelCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 stargatePos;



void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f); 

	//uv coordinates of every fragment of the model
	vec3 ndc = gl_Position.xyz / gl_Position.w; //perspective divide/normalize
	vec2 viewportCoord = ndc.xy * 0.5 + 0.5; //ndc is -1 to 1 in GL. scale for 0 to 1
	viewportPixelCoord = viewportCoord;
	
	//center of model position so we can recenter the UV coordinates on the model itself and not relative to the screen
	vec4 Position =  projection * view * vec4(stargatePos, 1.0f);;
	ndc = Position.xyz / Position.w; //perspective divide/normalize
	PosViewportPixelCoord = ndc.xy * 0.5 + 0.5; //ndc is -1 to 1 in GL. scale for 0 to 1; 
}