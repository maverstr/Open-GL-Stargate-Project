#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec2 pos2D;

uniform sampler2D screenTexture;
uniform int grayscale;
uniform int kernel;
uniform int sharpen;
uniform int blur;
uniform int edgeDetection;

const float offset = 1.0 / 300.0;  

void main()
{	
	//dark blue border
	if(pos2D.x < 0.51 || pos2D.y < 0.52){ 
		FragColor = vec4(0.0f, 0.0f, 0.33f, 0.8f);
	}
	//POV in the upper right corner
	else{
		if(grayscale == 1){
		//realistic grayscale using human eye sensibility to green
		    FragColor = texture(screenTexture, TexCoords);
			float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
			FragColor = vec4(average, average, average, 1.0);
		}
		else if(kernel == 1){
		//use of kernels for post-proc effects
			vec2 offsets[9] = vec2[](
			vec2(-offset,  offset), // top-left
			vec2( 0.0f,    offset), // top-center
			vec2( offset,  offset), // top-right
			vec2(-offset,  0.0f),   // center-left
			vec2( 0.0f,    0.0f),   // center-center
			vec2( offset,  0.0f),   // center-right
			vec2(-offset, -offset), // bottom-left
			vec2( 0.0f,   -offset), // bottom-center
			vec2( offset, -offset)  // bottom-right    
			);

			float kernel[9] = float[]( //default kernel
				1, 1, 1,
				1, 1, 1,
				1, 1, 1
			);

			if(sharpen == 1){
			kernel = float[](
				-1, -1, -1,
				-1,  9, -1,
				-1, -1, -1
			);
			  }

			else if(blur == 1){
				kernel = float[](
				1.0 / 16, 2.0 / 16, 1.0 / 16,
				2.0 / 16, 4.0 / 16, 2.0 / 16,
				1.0 / 16, 2.0 / 16, 1.0 / 16  
				);
			}

			else if(edgeDetection == 1){
			kernel = float[](
				1, 1, 1,
				1,  -8, 1,
				1, 1, 1
			);
			}
			vec3 sampleTex[9];
			for(int i = 0; i < 9; i++)
			{
				sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
			}
			vec3 col = vec3(0.0);
			for(int i = 0; i < 9; i++)
				col += sampleTex[i] * kernel[i];
    
			FragColor = vec4(col, 1.0);
		}

		else{
		//default pov without effect
		FragColor = texture(screenTexture, TexCoords);
		}
	}
}