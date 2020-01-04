#version 330 core
out vec4 FragColor;
struct Material{
	sampler2D texture_diffuse1;
};
uniform Material material;
uniform float time;
uniform float angle;
uniform float cameraFov;
uniform float random;
uniform float aspect; 

in vec3 FragPos;

in vec2 TexCoords;
in vec2 viewportPixelCoord;
in vec2 sunPosViewportPixelCoord; 

float snoise(vec3 uv, float res)	// by "trisomie21" on ShaderToy
{
	const vec3 s = vec3(1e0, 1e2, 1e4);
	
	uv *=  res *0.1;

	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;
	
	vec3 f = fract(uv); f = f*f*(3.0-2.0*f);
	
	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      	  uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);
	
	vec4 r = fract(sin(v*1e-3)*1e5);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v + uv1.z - uv0.z)*1e-3)*1e5);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.-1.;
}


void main()
{
	//inspired by different shaders on ShaderToy, heavily modified to work in GLSL and tweaked with experimental values using Render doc shader "real-time" editor
	//special credits to "trisomie21"
	float Random = random;
	float brightness	= Random *0.4;
	float radius		= 0.24 + brightness * 0.2;
	float invRadius 	= 1.0/radius;
	
	vec3 orange			= vec3( 0.8, 0.65, 0.3 );
	vec3 almostRed		= vec3(1.0f, 0.1f, 0.05f) * 0.00001;
	vec3 orangeRed		= vec3( 0.8, 0.20, 0.1 );
	float time = time;
	float aspect	=  aspect;
	vec2 uv			=  (viewportPixelCoord - sunPosViewportPixelCoord) *(cameraFov/angle) *1.15; //screen coordinates, need to correct for camera FOV
	vec2 p 			=  uv;
	p.x *= aspect;

	float fade		= pow( length( 2.0 * p ), 0.4 );
	float fVal1		= 1.1 - fade;
	float fVal2		= 1.1 - fade;
	
	float angle		= atan( p.x, p.y )/6.2832;
	float dist		= length(p);
	vec3 coord		= vec3( angle, dist, time * 0.1 );

	float newTime1	= abs( snoise( coord + vec3( 0.0, -time * ( 0.35 + brightness * 0.001 ), time * 0.015 ), 15.0 ) );
	float newTime2	= abs( snoise( coord + vec3( 0.0, -time * ( 0.15 + brightness * 0.001 ), time * 0.015 ), 45.0 ) );	
	for( int i=1; i<=7; i++ ){
		float power = pow( 2.0, float(i + 1) );
		fVal1 += ( 0.5 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 10.0 ) * ( newTime1 + 1.0 ) ) );
		fVal2 += ( 0.5 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 25.0 ) * ( newTime2 + 1.0 ) ) );
	}
	
	float corona		= pow( fVal1 * max( 1.1 - fade, 0.0 ), 2.0 ) * 20.0;
	corona				+= pow( fVal2 * max( 1.1 - fade, 0.0 ), 2.0 ) * 100.0;
	corona				*= 1.2 - newTime1;


	vec2 sp = uv; // correction to recenter if needed
	sp.x *= aspect;
	sp *= ( 2.0 - brightness ); //increase of brightness
  	float r = dot(sp,sp)*2.0; //use of screen coord to  determine the circle radius at each point
	float f = (1.0-sqrt(abs(1.0-r)))/(r) + brightness * 0.8; //normalize and positive only of r + increase according to brightness

	FragColor.rgb	= vec3( f * ( 0.75 + brightness * 0.8 ) * orange ) ; //drawing the basic circle with varying brightness

	if( dist < (radius*1.4)){
		FragColor = mix(FragColor, texture(material.texture_diffuse1, TexCoords), 0.7); //adding the texture in the sun core
	}
	if (dist < (radius *1.8)){
		corona			*= pow( dist * invRadius, 25.0 );
	}

	FragColor.rgb += corona * almostRed;
	float starGlow	= min( max( 1.0 - dist * ( 1.0 - brightness ), 0.0 ), 1.0 );
	FragColor.rgb += starGlow * orangeRed;
	FragColor.a = pow(1/exp(r-1.2),4); //progressive peripheral transparency
}