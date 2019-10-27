#version 410 core

out vec4 color;
in vec3 colorOut;

void main(){
	color = vec4(colorOut.x, colorOut.y, colorOut.z, 1.0f);
}