#version 330 core
out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform float time;
uniform float angle;
uniform float cameraFov;
uniform float aspect;

in vec3 FragPos;
in vec2 TexCoords;
in vec2 viewportPixelCoord;
in vec2 PosViewportPixelCoord; 

//shader for the watter ripple effect in the stargate
//inspired by many shaders on ShaderToy.com, special credits to "sinewaves" for the functions belows
//heavily tweaked and modified 


float hash(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float ang(vec2 uv, vec2 center){
	return atan((uv.y-center.y),(uv.x-center.x));
}

float spir(vec2 uv, vec2 loc, float time){
    float dist1=length(uv-loc);
    float dist2=dist1*dist1;
    float layer6=sin((ang(uv,loc)+dist2-time)*6.0);
    layer6 = layer6*dist1;
    return layer6;
}

float ripl(vec2 uv, vec2 loc, float speed, float frequency, float time){
    return sin(time*speed-length(uv-loc)*frequency);
}

float height(in vec2 uv, float time){
    float layer1=sin(time*8.54-sin(length(uv-vec2(-0.61,-0.65)))*55.0);
    float layer2=sin(time*7.13-sin(length(uv-vec2(0.85,0.82)))*43.0);
    float layer3=sin(time*7.92-sin(length(uv-vec2(-0.64,0.8)))*42.5);
    float layer4=sin(time*6.71-sin(length(uv-vec2(0.85,-0.75)))*47.2);

    float spiral=spir(uv,vec2(0.0,0.0), time); //shifting of the spiral if needed but centered gives best results
    spiral*=3.0;
    
    float temp = layer1+layer2+layer3+layer4+spiral;
    
    float b=smoothstep(-1.5,6.0,temp);
    return b*3.0;
}

void main()
{
//basically, the effect is the sum of 4 circular waves in the corners of the quad used as a double plane, and a spiral. Additional background light texture
	float time = time;
	float aspect	= aspect;
	vec2 uv			=  (viewportPixelCoord - PosViewportPixelCoord) *(cameraFov/angle) *0.3; //screen coordinates, need to correct for camera FOV

	//float waveHeight=0.2+height(uv, time); //base light amount
	//better mapping found online
    float waveHeight=(height(uv, time)*.4)+(.2/length(uv-(sin(time)*.5+.5)+vec2(height(uv+vec2(.01,.0), time)-height(uv+vec2(-.01,0.), time),height(uv+vec2(.0,.01), time)-height(uv+vec2(0.,-.01), time))));

    vec4 color=vec4(waveHeight*0.3,waveHeight*0.5,waveHeight, 1.0f); //color adjustment for blueiesh
    
    FragColor = mix(color,texture(texture_diffuse1, TexCoords), 0.15);
}