#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 colorIn;

out vec3 colorOut;

uniform mat4 MVP;

void main(){
gl_Position = MVP * vec4(position.x, position.y, position.z, 1.0f);
colorOut = vec3(colorIn.x, colorIn.y, colorIn.z);
}