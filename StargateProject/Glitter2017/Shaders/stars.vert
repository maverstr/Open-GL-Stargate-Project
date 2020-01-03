#version 410 core

layout(location = 0 ) in vec3 position;
layout(location = 1) in vec4 starInfo;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
gl_Position = projection * view * model * vec4(position + starInfo.xyz, 1.0f);
gl_PointSize = starInfo.w;
}