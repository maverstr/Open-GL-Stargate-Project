#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 Color;

out vec3 color;

uniform mat4 MVP;

void main(){
gl_Position = MVP * vec4(position.x, position.y, position.z, 1.0f);
color = vec4(Color, 1.0f);
}